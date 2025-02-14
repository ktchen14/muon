#include "boolean_expr.h"

#include "engine.h"
#include "node.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

const mu_boolean_expr_t *mu_boolean_expr(mu_engine_t *engine, _Bool data) {
  size_t size = sizeof(mu_boolean_expr_t);

  mu_boolean_expr_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_boolean_expr_t) {
    .as_expr.kind = MU_BOOLEAN_EXPR, .data = data,
  };

  return assign_node(engine, result);
}

#include "debug.h"

void mu_boolean_expr_debug(const mu_boolean_expr_t *expr) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "BooleanExpr#%zu(data = %s)",
    expr->as_stator.id, expr->data ? "true" : "false");
  debug_node_type(&expr->as_node);
  putc('\n', stderr);
}
