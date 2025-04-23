#include "common.h"

#include "../common.h"
#include "../inductor.h"

#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/Types.h>

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

/// @internal Assign @a length to @a result. Return 1 on overflow.
static inline _Bool llvm_length_overflow(size_t length, unsigned int *result) {
  if (length > UINT_MAX)
    return 1;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
  *result = length;
#pragma GCC diagnostic pop

  return 0;
}

__attribute__((nonnull, returns_nonnull))
static LLVMTypeRef evince_result(const author_t *author, const mu_type_t *type) {
  assert(type->id < author->type_length);
  LLVMTypeRef result = author->type_to_type[type->id];
  assert(result != NULL);
  return result;
}

/// @internal Return @c i1
__attribute__((nonnull)) static LLVMTypeRef boolean_type_emit(
    author_t *author, const mu_core_type_t *type) {
  return LLVMInt1Type();
}

/// @internal Return @c i64
__attribute__((nonnull)) static LLVMTypeRef integer_type_emit(
    author_t *author, const mu_core_type_t *type) {
  return LLVMInt64Type();
}

__attribute__((nonnull)) static LLVMTypeRef lambda_type_emit(
    author_t *author, const mu_core_type_t *type) {
  LLVMTypeRef argument_type = evince_result(author, type->argv[0]);
  LLVMTypeRef output_type = evince_result(author, type->argv[1]);
  return LLVMFunctionType(output_type, &argument_type, 1, 0);
}

/// @internal Return <tt>{ ... }</tt> where each type in ... is the type of the
/// argument in @c type->argv at the same index.
__attribute__((nonnull)) static LLVMTypeRef record_type_emit(
    author_t *author, const mu_core_type_t *type) {
  const mu_core_t *core = type->core;

  unsigned int argc;
  if (rare(llvm_length_overflow(core->argc, &argc)))
    return errno = EOVERFLOW, NULL;

  // Don't malloc a buffer if argc < 256
  if (core->argc < 256) {
    LLVMTypeRef argv[256];

    for (size_t i = 0; i < core->argc; i++)
      argv[i] = evince_result(author, type->argv[i]);

    return LLVMStructType(argv, argc, 0);
  }

  size_t size;
  if (rare(__builtin_mul_overflow(sizeof(LLVMTypeRef), core->argc, &size)))
    return errno = ENOMEM, NULL;

  LLVMTypeRef *argv;
  if ((argv = malloc(size)) == NULL)
    return NULL;

  for (size_t i = 0; i < core->argc; i++)
    argv[i] = evince_result(author, type->argv[i]);

  LLVMTypeRef result = LLVMStructType(argv, argc, 0);
  free(argv);
  return result;
}

/// @internal Return <tt>{ i64, ptr }</tt>
__attribute__((nonnull)) static LLVMTypeRef vector_type_emit(
    author_t *author, const mu_core_type_t *type) {
  LLVMTypeRef matter_type = evince_result(author, type->argv[0]);

  LLVMTypeRef allocation_type;
  if ((allocation_type = LLVMPointerType(matter_type, 0)) == NULL)
    return NULL;

  LLVMTypeRef argv[] = { LLVMInt64Type(), allocation_type };
  return LLVMStructType(argv, 2, 0);
}

__attribute__((nonnull)) static LLVMTypeRef custom_type_emit(
    author_t *author, const mu_core_type_t *type) {
  return LLVMInt64Type();
}

__attribute__((nonnull)) static LLVMTypeRef core_type_emit(
    author_t *author, const mu_core_type_t *type) {
  switch (type->core->kind) {
    case MU_BOOLEAN_CORE: return boolean_type_emit(author, type);
    case MU_CUSTOM_CORE:  return custom_type_emit(author, type);
    case MU_INTEGER_CORE: return integer_type_emit(author, type);
    case MU_LAMBDA_CORE:  return lambda_type_emit(author, type);
    case MU_RECORD_CORE:  return record_type_emit(author, type);
    case MU_VECTOR_CORE:  return vector_type_emit(author, type);
  }
  __builtin_unreachable();
}

__attribute__((nonnull)) static LLVMTypeRef join_type_emit(
    author_t *author, const mu_join_type_t *type) {
  size_t result_size = 0;
  for (size_t i = 0; i < type->argc; i++) {
    LLVMTypeRef argument = evince_result(author, type->argv[i]);
    size_t size = LLVMABISizeOfType(author->layout, argument);
    result_size = maximum(result_size, size);
  }

  LLVMTypeRef byte_type = LLVMInt8Type();
  LLVMTypeRef data_type;
  if ((data_type = LLVMArrayType2(byte_type, result_size)) == NULL)
    return NULL;

  LLVMTypeRef argv[] = { LLVMInt64Type(), data_type };
  return LLVMStructType(argv, 2, 0);
}

// NOLINTNEXTLINE: misc-no-recursion
__attribute__((nonnull)) static LLVMTypeRef type_emit(
    author_t *author, const mu_type_t *type) {
  switch ON_ABSTRACT_OBJECT(type) {
    case IS_KIND_OF(core_type):
      return core_type_emit(author, core_type);

    case MU_SCHEME_TYPE:
      abort();

    case IS_KIND_OF(variable_type):
      assert(variable_type->solution != NULL);
      return type_emit(author, variable_type->solution);

    case IS_KIND_OF(join_type):
      return join_type_emit(author, join_type);
  }
  __builtin_unreachable();
}

LLVMTypeRef get_type(author_t *author, const mu_type_t *root) {
  assert(root->id < author->type_length);

  const mu_type_t *type = root, *next;
  do {
    _Bool next_charge;
    while ((next = type_next(type, &next_charge)) != NULL) {
      assert(next->id < author->type_length);

      LLVMTypeRef answer = author->type_to_type[next->id];
      if (answer != NULL)
        continue;

      type = type_continue(type, next, next_charge);
    }

    LLVMTypeRef answer;
    if ((answer = type_emit(author, type)) == NULL)
      return NULL;
    author->type_to_type[type->id] = answer;
  } while ((type = type_return(type)) != NULL);

  return evince_result(author, root);
}
