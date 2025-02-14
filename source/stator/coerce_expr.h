#ifndef MU_STATOR_COERCE_EXPR_I
#define MU_STATOR_COERCE_EXPR_I

#include <muon/stator/coerce_expr.h>  // IWYU pragma: export

#include "abstract_node.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline const mu_node_t *coerce_expr_at(
    const mu_coerce_expr_t *expr, size_t i) {
  return i == 0 ? &expr->matter->as_node : NULL;
}

#endif /* MU_STATOR_COERCE_EXPR_I */
