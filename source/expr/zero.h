#ifndef MU_EXPR_ZERO_I
#define MU_EXPR_ZERO_I

#include <muon/expr/zero.h>  // IWYU pragma: export

#include "common.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline size_t zero_expr_size(const mu_zero_expr_t *expr) {
  return sizeof(mu_zero_expr_t);
}

void zero_expr_debug(const mu_zero_expr_t *expr)
  __attribute__((nonnull));

#endif /* MU_EXPR_ZERO_I */
