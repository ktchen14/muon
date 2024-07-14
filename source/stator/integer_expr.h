#ifndef MU_STATOR_INTEGER_EXPR_I
#define MU_STATOR_INTEGER_EXPR_I

#include <muon/stator/integer_expr.h>  // IWYU pragma: export

#include "abstract_node.h"
#include "abstract_type.h"
#include "integer_type.h"
#include "../inductor.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline const mu_node_t *integer_expr_at(
    const mu_integer_expr_t *expr, size_t i) {
  return NULL;
}

__attribute__((nonnull))
static inline const mu_type_t *integer_expr_induce(
    const mu_integer_expr_t *expr, inductor_t *inductor) {
  const mu_integer_type_t *integer_type;
  if ((integer_type = mu_integer_type(inductor->engine)) == NULL)
    return NULL;
  return &integer_type->as_type;
}

#endif /* MU_STATOR_INTEGER_EXPR_I */
