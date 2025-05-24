#include "author.h"
#include "stator.h"

#include <llvm-c/Core.h>
#include <llvm-c/Types.h>

#include <stdint.h>
#include <stdio.h>

uint64_t handle_list(struct { uint64_t length; void *data; } argument) {
  uint64_t *data = argument.data;
  uint64_t result = 0;
  for (size_t i = 0; i < argument.length; i++)
    result += data[i];
  return result;
}

__attribute__((nonnull))
static LLVMValueRef handle_list_emit(author_t *author, const mu_native_expr_t *expr) {
  LLVMTypeRef integer_type = LLVMInt64Type();
  LLVMTypeRef argv[] = { author->vector_type };
  LLVMTypeRef type = LLVMFunctionType(integer_type, argv, 1, 0);

  LLVMValueRef handle_list = LLVMAddFunction(author->module, "handle_list", type);
  LLVMSetLinkage(handle_list, LLVMExternalLinkage);

  return handle_list;
}

LLVMValueRef standard_native_expr_emit(
    author_t *author, const mu_native_expr_t *expr) {
  return handle_list_emit(author, expr);
}
