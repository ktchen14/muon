#include "author.h"
#include "stator.h"

#include <llvm-c/Core.h>
#include <llvm-c/Types.h>

#include <stdio.h>

static char name[256];

__attribute__((nonnull))
static LLVMValueRef handle_list_emit(author_t *author, const mu_native_expr_t *expr) {
  LLVMTypeRef integer_type = LLVMInt64Type();
  LLVMTypeRef argv[] = { author->vector_type };
  LLVMTypeRef type = LLVMFunctionType(integer_type, argv, 1, 0);

  snprintf(name, sizeof(name), "native.%zu", expr->as_node.id);
  LLVMValueRef handle_list = LLVMAddFunction(author->module, name, type);
  LLVMSetLinkage(handle_list, LLVMExternalLinkage);

  return handle_list;
}

LLVMValueRef standard_native_expr_emit(
    author_t *author, const mu_native_expr_t *expr) {
  return handle_list_emit(author, expr);
}
