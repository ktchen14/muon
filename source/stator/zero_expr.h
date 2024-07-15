#ifndef MU_STATOR_ZERO_EXPR_I
#define MU_STATOR_ZERO_EXPR_I

#include <muon/stator/zero_expr.h>  // IWYU pragma: export

#include "abstract_node.h"
#include "abstract_type.h"
#include "variable_type.h"
#include "../inductor.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline const mu_node_t *zero_expr_at(
    const mu_zero_expr_t *expr, size_t i) {
  return NULL;
}

__attribute__((nonnull))
static inline const mu_type_t *zero_expr_induce(
    const mu_zero_expr_t *expr, inductor_t *inductor) {
  const mu_variable_type_t *open_type;
  if ((open_type = mu_open_type(inductor->engine)) == NULL)
    return NULL;
  return &open_type->as_type;
}

#endif /* MU_STATOR_ZERO_EXPR_I */
