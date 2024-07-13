#ifndef MU_NODE_INTEGER_EXPR_I
#define MU_NODE_INTEGER_EXPR_I

#include <muon/node/integer_expr.h>  // IWYU pragma: export

#include "common.h"
#include "../inductor.h"
#include "../type.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline const mu_node_t *integer_expr_at(
    const mu_integer_expr_t *expr, size_t i) {
  return NULL;
}

__attribute__((nonnull))
static inline inductor_t *integer_expr_induce(
    const mu_integer_expr_t *expr, inductor_t *inductor) {
  const mu_integer_type_t *integer_type;
  if ((integer_type = mu_integer_type(inductor->engine)) == NULL)
    return NULL;

  const mu_type_t *type = &integer_type->as_type;
  const mu_node_t *node = &expr->as_node;
  return inductor_equate_node_type(inductor, node, type);
}

#endif /* MU_NODE_INTEGER_EXPR_I */
