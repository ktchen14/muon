#ifndef MU_STANDARD_I
#define MU_STANDARD_I

#include "author.h"
#include "engine.h"

#include <llvm-c/Types.h>

LLVMValueRef standard_native_expr_emit(
    author_t *author, mu_native_expr_t *expr)
  __attribute__((nonnull));

#endif /* MU_STANDARD_I */
