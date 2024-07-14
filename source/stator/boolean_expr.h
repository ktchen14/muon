#ifndef MU_STATOR_BOOLEAN_EXPR_I
#define MU_STATOR_BOOLEAN_EXPR_I

#include <muon/stator/boolean_expr.h>  // IWYU pragma: export

#include "node.h"
#include "boolean_type.h"
#include "type.h"
#include "../inductor.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline const mu_node_t *boolean_expr_at(
    const mu_boolean_expr_t *expr, size_t i) {
  return NULL;
}

__attribute__((nonnull))
static inline inductor_t *boolean_expr_induce(
    const mu_boolean_expr_t *expr, inductor_t *inductor) {
  const mu_boolean_type_t *boolean_type;
  if ((boolean_type = mu_boolean_type(inductor->engine)) == NULL)
    return NULL;

  const mu_type_t *type = &boolean_type->as_type;
  const mu_node_t *node = &expr->as_node;
  return inductor_equate_node_type(inductor, node, type);
}

#endif /* MU_STATOR_BOOLEAN_EXPR_I */
