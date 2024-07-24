#include "lambda_expr.h"

#include "engine.h"
#include "node.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

const mu_lambda_expr_t *mu_lambda_expr(
    mu_engine_t *engine, const mu_variable_view_t *argument, const mu_expr_t *matter) {
  assert(argument->as_stator.engine == engine);
  assert(matter->as_stator.engine == engine);

  size_t size = sizeof(mu_lambda_expr_t);

  mu_lambda_expr_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_lambda_expr_t) {
    .as_expr.kind = MU_LAMBDA_EXPR, .argument = argument, .matter = matter,
  };

  return assign_node(engine, result);
}

#include "debug.h"

void mu_lambda_expr_debug(const mu_lambda_expr_t *expr) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Lambda Expr #%zu:", expr->as_stator.id);
  debug_node_type(&expr->as_node);
  putc('\n', stderr);

  WITH_DEBUG_INDENT() {
    WITH_DEBUG_NEGATE() {
      mu_view_debug(&expr->argument->as_view);
    }
    mu_expr_debug(expr->matter);
  }
}
