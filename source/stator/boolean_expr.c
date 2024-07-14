#include "boolean_expr.h"

#include "engine.h"
#include "node.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

const mu_boolean_expr_t *mu_boolean_expr(
    mu_engine_t *engine, _Bool data, const mu_node_source_t *source) {
  size_t size = sizeof(mu_boolean_expr_t);

  mu_boolean_expr_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_boolean_expr_t) {
    .as_expr.kind = MU_BOOLEAN_EXPR, .data = data,
  };

  if (source != NULL)
    result->as_node.source = *source;

  return assign_node(engine, result);
}

void mu_boolean_expr_debug(const mu_boolean_expr_t *expr) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Boolean Expr #%zu: data = %s\n",
    expr->as_stator.id, expr->data ? "true" : "false");
}
