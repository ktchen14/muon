#ifndef MU_STATOR_NAME_EXPR_I
#define MU_STATOR_NAME_EXPR_I

#include <muon/stator/name_expr.h>  // IWYU pragma: export

#include "abstract_node.h"

#include <assert.h>
#include <stddef.h>

__attribute__((const, nonnull))
static inline const mu_node_t *name_expr_at(
    const mu_name_expr_t *expr, size_t i) {
  return NULL;
}

#endif /* MU_STATOR_NAME_EXPR_I */
