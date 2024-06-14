#ifndef MU_EXPR_VECTOR_I
#define MU_EXPR_VECTOR_I

#include <muon/expr/vector.h>  // IWYU pragma: export

#include "common.h"

__attribute__((const, nonnull))
static inline size_t vector_expr_size(const mu_vector_expr_t *expr) {
  return extant_size(mu_vector_expr_t, argv, expr->argc);
}

#endif /* MU_EXPR_VECTOR_I */
