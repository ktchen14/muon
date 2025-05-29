#include "common.h"

#include "../common.h"
#include "../inductor.h"

#include <llvm-c/Types.h>
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

__attribute__((nonnull)) static LLVMValueRef id_coercion_emit(
    author_t *author, const mu_id_coercion_t *coercion, info_t info) {
  return info.source;
}

__attribute__((nonnull)) static LLVMValueRef edge_coercion_emit(
    author_t *author, const mu_edge_coercion_t *coercion, info_t info) {
  abort();
}

__attribute__((nonnull)) static LLVMValueRef slot_coercion_emit(
    author_t *author, const mu_slot_coercion_t *coercion, info_t info) {
  abort();
}

__attribute__((nonnull)) static LLVMValueRef indirect_coercion_emit(
    author_t *author, const mu_indirect_coercion_t *coercion, info_t info) {
  MuonType *middle_type = coercion->head->target;
  LLVMTypeRef middle_llvm_type;
  if ((middle_llvm_type = get_type(author, middle_type)) == NULL)
    return NULL;

  LLVMValueRef middle;
  if ((middle = coercion_emit(author, coercion->head, info)) == NULL)
    return NULL;

  info = (info_t) { .source_type = middle_type, .source = middle };
  return coercion_emit(author, coercion->tail, info);
}

__attribute__((nonnull)) static LLVMValueRef variance_coercion_emit(
    author_t *author, const mu_variance_coercion_t *coercion, info_t info) {
  switch ON_ABSTRACT_OBJECT(coercion->core) {
    case MU_BOOLEAN_CORE:
    case MU_INTEGER_CORE:
      return info.source;

    case MU_LAMBDA_CORE:
    case MU_RECORD_CORE:
      return info.source;

    case MU_VECTOR_CORE: {
      return info.source;
    }

    case MU_CUSTOM_CORE:
      return info.source;
  }
}

__attribute__((nonnull)) static LLVMValueRef instance_coercion_emit(
    author_t *author, const mu_instance_coercion_t *coercion, info_t info) {
  abort();
}

__attribute__((nonnull)) static LLVMValueRef unscheme_coercion_emit(
    author_t *author, const mu_unscheme_coercion_t *coercion, info_t info) {
  abort();
}

__attribute__((nonnull)) static LLVMValueRef join_coercion_emit(
    author_t *author, const mu_join_coercion_t *coercion, info_t info) {
  LLVMBuilderRef tail = author->tail;

  LLVMTypeRef target_type;
  if ((target_type = get_type(author, coercion->as_coercion.target)) == NULL)
    return NULL;

  // %result = <target_type>
  LLVMValueRef result;
  if ((result = LLVMGetPoison(target_type)) == NULL)
    return NULL;

  // %discriminant = i64 <coercion->i>
  LLVMValueRef discriminant;
  if ((discriminant = LLVMConstInt(LLVMInt64Type(), coercion->i, 0)) == NULL)
    return NULL;

  // %result = insertvalue <info.target_type> %result, %discriminant, 0
  if ((result = LLVMBuildInsertValue(tail, result, discriminant, 0, "")) == NULL)
    return NULL;

  LLVMTypeRef data_type = LLVMStructGetTypeAtIndex(target_type, 1);

  // %allocation = alloca <data_type>
  LLVMValueRef allocation;
  if ((allocation = LLVMBuildAlloca(tail, data_type, "")) == NULL)
    return NULL;

  // store <typeof(info.source)> <info.source>, %allocation
  if (LLVMBuildStore(tail, info.source, allocation) == NULL)
    return NULL;

  // %data = load <data_type>, %allocation
  LLVMValueRef data;
  if ((data = LLVMBuildLoad2(tail, data_type, allocation, "")) == NULL)
    return NULL;

  // %result = insertvalue <info.target_type> %result, %data, 1
  if ((result = LLVMBuildInsertValue(tail, result, data, 1, "")) == NULL)
    return NULL;

  return result;
}

__attribute__((nonnull)) static LLVMValueRef unjoin_coercion_emit(
    author_t *author, const mu_unjoin_coercion_t *coercion, info_t info) {
  // Ensure that the source type is a join type
  MuonJoinType *source_type = mu_type_cast(info.source_type, source_type);
  assert(source_type != NULL);

  unsigned int argc;
  if (rare(llvm_length_overflow(coercion->argc, &argc)))
    return NULL;

  LLVMTypeRef llvm_source_type = LLVMTypeOf(info.source);
  LLVMTypeRef data_type = LLVMStructGetTypeAtIndex(llvm_source_type, 1);

  // %discriminant = i64 <info.source>
  LLVMValueRef discriminant = LLVMBuildExtractValue(author->tail, info.source, 0, "");

  // %allocation = alloca <data_type>
  LLVMValueRef allocation = LLVMBuildAlloca(author->tail, data_type, "");

  // %source_data = extractvalue { i64, <data_type> } <info.source>, 1
  LLVMValueRef source_data = LLVMBuildExtractValue(author->tail, info.source, 1, "");

  // store <data_type> %source_data, %allocation
  LLVMBuildStore(author->tail, source_data, allocation);

  // %none:
  LLVMBasicBlockRef none = LLVMAppendBasicBlock(author->lambda, "");

  // switch i64 %discriminant, label %none, ...
  LLVMValueRef jump = LLVMBuildSwitch(author->tail, discriminant, none, argc);

  // unreachable (in %none)
  LLVMPositionBuilderAtEnd(author->tail, none);
  LLVMBuildUnreachable(author->tail);

  // %next:
  LLVMBasicBlockRef next = LLVMAppendBasicBlock(author->lambda, "");

  LLVMTypeRef target_type;
  if ((target_type = get_type(author, coercion->as_coercion.target)) == NULL)
    return NULL;

  // %result = phi <target_type>, ...
  LLVMPositionBuilderAtEnd(author->tail, next);
  LLVMValueRef result = LLVMBuildPhi(author->tail, target_type, "");

  // For each discriminant, append a basic block that will load from the
  // allocation as the appropriate type, then emit the relevant coercion against
  // the loaded data.
  for (size_t i = 0; i < coercion->argc; i++) {
    // TODO: fix this
    char *name;
    if (asprintf(&name, "unjoin.discriminant.%zu", i) == -1)
      return NULL;

    // %branch:
    LLVMBasicBlockRef branch = LLVMAppendBasicBlock(author->lambda, name);
    LLVMPositionBuilderAtEnd(author->tail, branch);

    // %number = i64 <i>
    LLVMValueRef number = LLVMConstInt(LLVMInt64Type(), i, 0);

    // switch i64 %discriminant, ... [... i64 %number, label %branch ...]
    LLVMAddCase(jump, number, branch);

    LLVMTypeRef branch_type;
    if ((branch_type = get_type(author, source_type->argv[i])) == NULL)
      return NULL;

    // %data = load <branch_type>, %allocation
    LLVMValueRef data = LLVMBuildLoad2(author->tail, branch_type, allocation, "");

    info_t info = { .source_type = source_type->argv[i], .source = data };
    LLVMValueRef branch_result;
    if ((branch_result = coercion_emit(author, coercion->argv[i], info)) == NULL)
      return NULL;

    // br %next
    LLVMBuildBr(author->tail, next);

    LLVMAddIncoming(result, &branch_result, &branch, 1);
  }

  LLVMPositionBuilderAtEnd(author->tail, next);
  return result;
}

__attribute__((nonnull)) static LLVMValueRef meet_coercion_emit(
    author_t *author, const mu_meet_coercion_t *coercion, info_t info) {
  abort();
}

__attribute__((nonnull)) static LLVMValueRef unmeet_coercion_emit(
    author_t *author, const mu_unmeet_coercion_t *coercion, info_t info) {
  abort();
}

LLVMValueRef coercion_emit(author_t *author, const mu_coercion_t *coercion, info_t info) {
  switch (coercion->kind) {
#define MU_EMIT(lower, upper, t) case MU_##upper##_COERCION: \
      return lower##_coercion_emit( \
          author, (const mu_##lower##_coercion_t *) coercion, info);
    MU_EACH_COERCION_KIND(MU_EMIT);
#undef MU_EMIT

    default: return SKIP;
  }
}
