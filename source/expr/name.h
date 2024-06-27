#ifndef MU_EXPR_NAME_I
#define MU_EXPR_NAME_I

#include <muon/expr/name.h>  // IWYU pragma: export

#include "common.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline size_t name_expr_size(const mu_name_expr_t *expr) {
  return sizeof(mu_name_expr_t);
}

__attribute__((const, nonnull))
static inline const mu_node_t *name_expr_at(
    const mu_name_expr_t *expr, size_t i) {
  return NULL;
}

#endif /* MU_EXPR_NAME_I */
