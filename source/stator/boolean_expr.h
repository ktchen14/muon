#ifndef MU_STATOR_BOOLEAN_EXPR_I
#define MU_STATOR_BOOLEAN_EXPR_I

#include <muon/stator/boolean_expr.h>  // IWYU pragma: export

#include "abstract_node.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline const mu_node_t *boolean_expr_at(
    const mu_boolean_expr_t *expr, size_t i) {
  return NULL;
}

#endif /* MU_STATOR_BOOLEAN_EXPR_I */
