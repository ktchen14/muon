#include "invoke_expr.h"

#include "engine.h"
#include "node.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

const mu_invoke_expr_t *mu_invoke_expr(
    mu_engine_t *engine, const mu_expr_t *lambda, const mu_expr_t *matter) {
  assert(lambda->as_stator.engine == engine);
  assert(matter->as_stator.engine == engine);

  size_t size = sizeof(mu_invoke_expr_t);

  mu_invoke_expr_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_invoke_expr_t) {
    .as_expr.kind = MU_INVOKE_EXPR, .lambda = lambda, .matter = matter,
  };

  return assign_node(engine, result);
}

#include "debug.h"

void mu_invoke_expr_debug(const mu_invoke_expr_t *expr) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "InvokeExpr#%zu", expr->as_stator.id);
  debug_node_type(&expr->as_node);
  putc('\n', stderr);

  WITH_DEBUG_INDENT() {
    mu_expr_debug(expr->lambda);
    mu_expr_debug(expr->matter);
  }
}
