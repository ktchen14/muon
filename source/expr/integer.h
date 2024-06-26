#ifndef MU_EXPR_INTEGER_I
#define MU_EXPR_INTEGER_I

#include <muon/expr/integer.h>  // IWYU pragma: export

#include "common.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline size_t integer_expr_size(const mu_integer_expr_t *expr) {
  return sizeof(mu_integer_expr_t);
}

void integer_expr_debug(const mu_integer_expr_t *expr)
  __attribute__((nonnull));

#endif /* MU_EXPR_INTEGER_I */
