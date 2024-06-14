#include "zero.h"

#include "../common.h"
#include "../engine.h"

#include <stddef.h>

const mu_zero_expr_t *mu_zero_expr(mu_engine_t *engine) {
  size_t size = sizeof(mu_zero_expr_t);

  mu_zero_expr_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_zero_expr_t) {
    .as_expr.kind = MU_ZERO_EXPR,
  };
  return engine_assign_concrete(engine, result);
}
