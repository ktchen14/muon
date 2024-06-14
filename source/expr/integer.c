#include "integer.h"

#include "../common.h"
#include "../engine.h"

const mu_integer_expr_t *mu_integer_expr(mu_engine_t *engine, uint64_t data) {
  size_t size = sizeof(mu_integer_expr_t);

  mu_integer_expr_t *result;
  if (rare((result = node_allocate(engine, size)) == NULL))
    return NULL;
  *result = (mu_integer_expr_t) { .data = data };
  return engine_assign_concrete(engine, result);
}
