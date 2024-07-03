#include "zero_expr.h"

#include "../engine.h"
#include "../node.h"

#include <stddef.h>
#include <stdio.h>

const mu_zero_expr_t *mu_zero_expr(mu_engine_t *engine) {
  size_t size = sizeof(mu_zero_expr_t);

  mu_zero_expr_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_zero_expr_t) {
    .as_expr.kind = MU_ZERO_EXPR,
  };
  return assign_node(engine, result);
}

void mu_zero_expr_debug(const mu_zero_expr_t *expr) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Zero Expr #%zu\n", expr->as_stator.id);
}
