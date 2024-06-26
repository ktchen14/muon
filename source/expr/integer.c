#include "integer.h"

#include "../engine.h"
#include "../expr.h"
#include "../status.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

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

void integer_expr_debug(const mu_integer_expr_t *expr) {
  fprintf(stderr, "Integer Expr #%zu: %" PRIu64 "\n",
      expr->as_stator.id, expr->data);
}
