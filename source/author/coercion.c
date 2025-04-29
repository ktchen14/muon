#include "common.h"

#include "../common.h"
#include "../inductor.h"
#include "../stator.h"

#include <llvm-c/Types.h>
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static LLVMValueRef SKIP = (void *) &(int) {1};

static LLVMValueRef evince_result(const author_t *author, const mu_node_t *node) {
  assert(node->id < author->node_length);
  LLVMValueRef result = author->node_to_value[node->id];
  assert(result != NULL);
  return result;
}

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
  const mu_type_t *middle_type = coercion->head->target;
  LLVMTypeRef middle_llvm_type;
  if ((middle_llvm_type = get_type(author, middle_type)) == NULL)
    return NULL;

  info_t single_info = {
    .source_muon_type = info.source_muon_type,
    .source_type = info.source_type,
    .source = info.source,
  };

  LLVMValueRef middle;
  if ((middle = coercion_emit(author, coercion->head, single_info)) == NULL)
    return NULL;

  single_info = (info_t) {
    .source_muon_type = middle_type,
    .source_type = middle_llvm_type,
    .source = middle,
  };

  return coercion_emit(author, coercion->tail, single_info);
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

  // %number = i64 <coercion->i>
  LLVMValueRef number;
  if ((number = LLVMConstInt(LLVMInt64Type(), coercion->i, 0)) == NULL)
    return NULL;

  // %result = insertvalue <info.target_type> %result, %number, 0
  if ((result = LLVMBuildInsertValue(tail, result, number, 0, "")) == NULL)
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
  const mu_join_type_t *source_type = mu_type_cast(info.source_muon_type, source_type);
  assert(source_type != NULL);

  LLVMBuilderRef tail = LLVMCreateBuilder();
  for (size_t i = 0; i < coercion->argc; i++) {
    // TODO: fix this
    char *name;
    if (asprintf(&name, "unjoin.discriminant.%zu", i) == -1)
      return NULL;

    LLVMBasicBlockRef bblock = LLVMAppendBasicBlock(author->lambda, name);
    LLVMPositionBuilderAtEnd(tail, bblock);
    author->tail = tail;

    info_t discriminant_info = info;
    info.source_muon_type = source_type->argv[i];
    info.source_type = get_type(author, info.source_muon_type);

    // TODO: fix info
    coercion_emit(author, coercion->argv[i], info);
  }
  abort();
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
