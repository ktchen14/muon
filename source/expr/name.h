#ifndef MU_EXPR_NAME_I
#define MU_EXPR_NAME_I

#include <muon/expr/name.h>  // IWYU pragma: export

#include "common.h"
#include "../menu.h"
#include "../type.h"

#include <assert.h>
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

__attribute__((nonnull))
static inline criteria_t *name_expr_induce(
    const mu_name_expr_t *expr,
    criteria_t *criteria,
    const induce_t *induce) {
  assert(expr->as_stator.engine == induce->engine);
  assert(induce->node_to_stmt != NULL);

  const mu_stmt_t *target;
  if ((target = induce->node_to_stmt[expr->as_stator.id]) == NULL)
    return criteria;

  constraint_t constraint = {
    .a.node = &expr->as_node, .b.node = &target->as_node,
  };
  return criteria_append(criteria, constraint);
}

#endif /* MU_EXPR_NAME_I */
