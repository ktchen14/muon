#ifndef MU_NODE_NAME_EXPR_I
#define MU_NODE_NAME_EXPR_I

#include <muon/node/name_expr.h>  // IWYU pragma: export

#include "common.h"
#include "../inductor.h"

#include <assert.h>
#include <stddef.h>

__attribute__((const, nonnull))
static inline const mu_node_t *name_expr_at(
    const mu_name_expr_t *expr, size_t i) {
  return NULL;
}

__attribute__((nonnull))
static inline inductor_t *name_expr_induce(
    const mu_name_expr_t *expr, inductor_t *inductor) {
  const mu_stmt_t *target;
  if ((target = inductor->node_to_stmt[expr->as_stator.id]) == NULL)
    return inductor;

  return inductor_equate_node_node(inductor, &expr->as_node, &target->as_node);
}

#endif /* MU_NODE_NAME_EXPR_I */
