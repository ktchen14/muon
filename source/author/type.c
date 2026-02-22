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

MUON_HINT(nonnull, returns_nonnull)
static LLVMTypeRef evince_result(const author_t *author, MuonType *type) {
  assert(type->id < author->type_length);
  LLVMTypeRef result = author->type_to_type[type->id];
  assert(result != NULL);
  return result;
}

/// @internal Return @c i1
MUON_HINT(nonnull) static LLVMTypeRef boolean_type_emit(
    author_t *author, MuonCoreType *type) {
  return author->bool_type;
}

/// @internal Return @c i64
MUON_HINT(nonnull) static LLVMTypeRef integer_type_emit(
    author_t *author, MuonCoreType *type) {
  return LLVMInt64Type();
}

MUON_HINT(nonnull) static LLVMTypeRef lambda_type_emit(
    author_t *author, MuonCoreType *type) {
  LLVMTypeRef argument_type = evince_result(author, type->argv[0]);
  LLVMTypeRef output_type = evince_result(author, type->argv[1]);
  return LLVMFunctionType(output_type, &argument_type, 1, 0);
}

/// @internal Return <tt>{ ... }</tt> where each type in ... is the type of the
/// argument in @c type->argv at the same index.
MUON_HINT(nonnull) static LLVMTypeRef record_type_emit(
    author_t *author, MuonCoreType *type) {
  const MuonCore *core = type->core;

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
MUON_HINT(nonnull) static LLVMTypeRef vector_type_emit(
    author_t *author, MuonCoreType *type) {
  return author->vector_type;
}

MUON_HINT(nonnull) static LLVMTypeRef custom_type_emit(
    author_t *author, MuonCoreType *type) {
  return LLVMInt64Type();
}

MUON_HINT(nonnull) static LLVMTypeRef join_type_emit(
    author_t *author, MuonJoinType *type) {
  if (type->argc == 0)
    abort();

  // LLVM doesn't have a native union type so to synthesize it, we'll represent
  // the data of the join type as an array [length x member_type]. The
  // member_type must have the largest alignemnt requirement of any argument to
  // the join type. The length should be such that the size of the array is
  // sufficient to hold any argument to the join type.

  LLVMTargetDataRef layout = author->layout;

  LLVMTypeRef member_type = evince_result(author, type->argv[0]);
  size_t member_size = LLVMABISizeOfType(layout, member_type);
  size_t maximum_unit = LLVMABIAlignmentOfType(layout, member_type);
  size_t maximum_size = member_size;

  for (size_t i = 1; i < type->argc; i++) {
    LLVMTypeRef argument_type = evince_result(author, type->argv[i]);
    size_t size = LLVMABISizeOfType(layout, argument_type);
    size_t unit = LLVMABIAlignmentOfType(layout, argument_type);

    // Record the largest size of an argument to the join type
    maximum_size = maximum(maximum_size, size);

    // Record the argument to the join type with the largest alignment
    // requirement. If multiple arguments tie, then record the one with the
    // smallest size.
    if (unit < maximum_unit || unit == maximum_unit && size >= member_size)
      continue;
    member_type = argument_type;
    member_size = size;
    maximum_unit = unit;
  }

  size_t length = maximum_size / member_size;
  if (maximum_size % member_size > 0)
    length++;

  LLVMTypeRef data_type;
  if ((data_type = LLVMArrayType2(member_type, length)) == NULL)
    return NULL;

  LLVMTypeRef argv[] = {LLVMInt64Type(), data_type};
  return LLVMStructType(argv, 2, 0);
}

// NOLINTNEXTLINE: misc-no-recursion
MUON_HINT(nonnull) static LLVMTypeRef type_emit(
    author_t *author, MuonType *type) {
  switch ON_ABSTRACT_OBJECT(type) {
    case IS_CONCRETE_TYPE(MuonCoreType * nominate(core_type))
      switch (core_type->core->kind) {
        case MUON_BOOLEAN_CORE:
          return boolean_type_emit(author, core_type);

        case MUON_CUSTOM_CORE:
          return custom_type_emit(author, core_type);

        case MUON_INTEGER_CORE:
          return integer_type_emit(author, core_type);

        case MUON_LAMBDA_CORE:
          return lambda_type_emit(author, core_type);

        case MUON_RECORD_CORE:
          return record_type_emit(author, core_type);

        case MUON_VECTOR_CORE:
          return vector_type_emit(author, core_type);
      }
      __builtin_unreachable();

    case MU_SCHEME_TYPE:
      abort();

    case IS_CONCRETE_TYPE(MuonVariableType * nominate(variable_type))
      assert(variable_type->solution != NULL);
      return type_emit(author, variable_type->solution);

    case IS_CONCRETE_TYPE(MuonJoinType * nominate(join_type))
      return join_type_emit(author, join_type);
  }
  __builtin_unreachable();
}

LLVMTypeRef get_type(author_t *author, MuonType *root) {
  assert(root->id < author->type_length);

  MuonType *type = root, *next;
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
