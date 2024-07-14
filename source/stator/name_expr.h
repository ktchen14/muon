#ifndef MU_STATOR_NAME_EXPR_I
#define MU_STATOR_NAME_EXPR_I

#include <muon/stator/name_expr.h>  // IWYU pragma: export

#include "abstract_node.h"
#include "abstract_type.h"
#include "variable_type.h"
#include "../inductor.h"

#include <assert.h>
#include <stddef.h>

__attribute__((const, nonnull))
static inline const mu_node_t *name_expr_at(
    const mu_name_expr_t *expr, size_t i) {
  return NULL;
}

__attribute__((nonnull))
static inline const mu_type_t *name_expr_induce(
    const mu_name_expr_t *expr, inductor_t *inductor) {
  const mu_stmt_t *target;
  if ((target = inductor->node_to_stmt[expr->as_stator.id]) == NULL) {
    const mu_variable_type_t *variable_type;
    if ((variable_type = mu_variable_type(inductor->engine)) == NULL)
      return NULL;
    return &variable_type->as_type;
  }

  const mu_type_t *type;
  if ((type = inductor_node_type(inductor, &target->as_node)) != NULL)
    return type;

  const mu_variable_type_t *variable_type;
  if ((variable_type = mu_variable_type(inductor->engine)) == NULL)
    return NULL;
  inductor->node_to_type[target->as_stator.id] = &variable_type->as_type;
  return &variable_type->as_type;
}

#endif /* MU_STATOR_NAME_EXPR_I */
