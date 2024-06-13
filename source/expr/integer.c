#include "integer.h"

#include "../common.h"
#include "../engine.h"

const mu_integer_expr_t *mu_integer_expr(mu_engine_t *engine, uint64_t data) {
  size_t size = sizeof(mu_integer_expr_t);

  mu_integer_expr_t *integer;
  if (rare((integer = engine_allocate(engine, size)) == NULL))
    return NULL;
  *integer = (mu_integer_expr_t) { .data = data };

  return (mu_integer_expr_t *) engine_register(engine, &integer->as_stator);
}
