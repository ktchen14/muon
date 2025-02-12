#include "name_expr.h"

#include "engine.h"
#include "name.h"
#include "node.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

const mu_name_expr_t *mu_name_expr(mu_engine_t *engine, const mu_name_t *name) {
  assert(name->as_stator.engine == engine);

  size_t size = sizeof(mu_name_expr_t);

  mu_name_expr_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_name_expr_t) {
    .as_expr.kind = MU_NAME_EXPR, .name = name,
  };

  return assign_node(engine, result);
}

#include "debug.h"

void mu_name_expr_debug(const mu_name_expr_t *expr) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Name Expr #%zu: ", expr->as_stator.id);
  mu_name_debug(expr->name);
  debug_node_type(&expr->as_node);
  putc('\n', stderr);
}
