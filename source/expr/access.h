#ifndef MU_EXPR_ACCESS_I
#define MU_EXPR_ACCESS_I

#include <muon/expr/access.h>  // IWYU pragma: export

#include "common.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline size_t access_expr_size(const mu_access_expr_t *expr) {
  return sizeof(mu_access_expr_t);
}

__attribute__((nonnull, pure))
static inline const mu_node_t *access_expr_at(
    const mu_access_expr_t *expr, size_t i) {
  return i == 0 ? &expr->matter->as_node : NULL;
}

#endif /* MU_EXPR_ACCESS_I */
