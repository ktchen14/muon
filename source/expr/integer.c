#include "integer.h"

#include "../common.h"
#include "../engine.h"
#include "../status.h"

#include <stddef.h>
#include <stdint.h>

const mu_integer_expr_t *mu_integer_expr(
    mu_engine_t *engine, uint64_t data, const mu_source_t *source) {
  size_t size = sizeof(mu_integer_expr_t);

  mu_integer_expr_t *result;
  if (rare((result = node_allocate(engine, size)) == NULL))
    return NULL;
  *result = (mu_integer_expr_t) {
    .as_expr.kind = MU_INTEGER_EXPR, .data = data,
  };

  if (source != NULL)
    result->as_node.source = *source;

  return engine_assign_concrete(engine, result);
}
