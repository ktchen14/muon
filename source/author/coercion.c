#include "common.h"

#include "../stator.h"
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

typedef struct {
  LLVMTypeRef source_type;
  LLVMTypeRef target_type;
  LLVMValueRef source;
} info_t;

LLVMValueRef coercion_emit(frame_t *frame, const mu_coercion_t *coercion, info_t info);

static LLVMValueRef SKIP = (void *) &(int) {1};

static LLVMValueRef evince_result(const frame_t *frame, const mu_node_t *node) {
  author_t *author = frame->author;
  assert(node->id < author->node_length);
  LLVMValueRef result = author->node_to_value[node->id];
  assert(result != NULL);
  return result;
}

__attribute__((nonnull)) static LLVMValueRef id_coercion_emit(
    frame_t *frame, const mu_id_coercion_t *coercion, info_t info) {
  return info.source;
}

__attribute__((nonnull)) static LLVMValueRef edge_coercion_emit(
    frame_t *frame, const mu_edge_coercion_t *coercion, info_t info) {
  abort();
}

__attribute__((nonnull)) static LLVMValueRef slot_coercion_emit(
    frame_t *frame, const mu_slot_coercion_t *coercion, info_t info) {
  abort();
}

__attribute__((nonnull)) static LLVMValueRef indirect_coercion_emit(
    frame_t *frame, const mu_indirect_coercion_t *coercion, info_t info) {
  abort();
  // TODO: fix this
  /* LLVMValueRef intermediate = coercion_emit(frame, coercion->head, info.source); */
  /* return coercion_emit(frame, coercion->tail, intermediate); */
}

__attribute__((nonnull)) static LLVMValueRef variance_coercion_emit(
    frame_t *frame, const mu_variance_coercion_t *coercion, info_t info) {
  abort();
}

__attribute__((nonnull)) static LLVMValueRef instance_coercion_emit(
    frame_t *frame, const mu_instance_coercion_t *coercion, info_t info) {
  abort();
}

__attribute__((nonnull)) static LLVMValueRef unscheme_coercion_emit(
    frame_t *frame, const mu_unscheme_coercion_t *coercion, info_t info) {
  abort();
}

__attribute__((nonnull)) static LLVMValueRef join_coercion_emit(
    frame_t *frame, const mu_join_coercion_t *coercion, info_t info) {
  LLVMBuilderRef b = frame->builder;

  // %result = <info.target_type>
  LLVMValueRef result;
  if ((result = LLVMGetPoison(info.target_type)) == NULL)
    return NULL;

  // %number = i64 <coercion->i>
  LLVMValueRef number;
  if ((number = LLVMConstInt(LLVMInt64Type(), coercion->i, 0)) == NULL)
    return NULL;

  // %result = insertvalue <info.target_type> %result, %number, 0
  if ((result = LLVMBuildInsertValue(b, result, number, 0, "")) == NULL)
    return NULL;

  LLVMTypeRef data_type = LLVMStructGetTypeAtIndex(info.target_type, 1);

  // %allocation = alloca <data_type>
  LLVMValueRef allocation;
  if ((allocation = LLVMBuildAlloca(b, data_type, "")) == NULL)
    return NULL;

  // store <typeof(info.source)> <info.source>, %allocation
  if (LLVMBuildStore(b, info.source, allocation) == NULL)
    return NULL;

  // %data = load <data_type>, %allocation
  LLVMValueRef data;
  if ((data = LLVMBuildLoad2(b, data_type, allocation, "")) == NULL)
    return NULL;

  // %result = insertvalue <info.target_type> %result, %data, 1
  if ((result = LLVMBuildInsertValue(b, result, data, 1, "")) == NULL)
    return NULL;

  return result;
}

__attribute__((nonnull)) static LLVMValueRef unjoin_coercion_emit(
    frame_t *frame, const mu_unjoin_coercion_t *coercion, info_t info) {
  abort();
}

__attribute__((nonnull)) static LLVMValueRef meet_coercion_emit(
    frame_t *frame, const mu_meet_coercion_t *coercion, info_t info) {
  abort();
}

__attribute__((nonnull)) static LLVMValueRef unmeet_coercion_emit(
    frame_t *frame, const mu_unmeet_coercion_t *coercion, info_t info) {
  abort();
}

LLVMValueRef coercion_emit(frame_t *frame, const mu_coercion_t *coercion, info_t info) {
  switch (coercion->kind) {
#define MU_EMIT(lower, upper, t) case MU_##upper##_COERCION: \
      return lower##_coercion_emit( \
          frame, (const mu_##lower##_coercion_t *) coercion, info);
    MU_EACH_COERCION_KIND(MU_EMIT);
#undef MU_EMIT

    default: return SKIP;
  }
}
