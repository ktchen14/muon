#ifndef MU_STATOR_ACCESS_EXPR_I
#define MU_STATOR_ACCESS_EXPR_I

#include <muon/stator/access_expr.h>  // IWYU pragma: export

#include "abstract_node.h"
#include "abstract_type.h"

#include <assert.h>
#include <stddef.h>

__attribute__((nonnull, pure))
static inline const mu_node_t *access_expr_at(
    const mu_access_expr_t *expr, size_t i) {
  return i == 0 ? &expr->matter->as_node : NULL;
}

#endif /* MU_STATOR_ACCESS_EXPR_I */
