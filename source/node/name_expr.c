#include "name_expr.h"

#include "../engine.h"
#include "../name.h"
#include "../node.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

const mu_name_expr_t *mu_name_expr(
    mu_engine_t *engine, const mu_name_t *name, const mu_source_t *source) {
  assert(name->as_stator.engine == engine);

  size_t size = sizeof(mu_name_expr_t);

  mu_name_expr_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_name_expr_t) {
    .as_expr.kind = MU_NAME_EXPR, .name = name,
  };

  if (source != NULL)
    result->as_node.source = *source;

  return node_assign(engine, result);
}

void mu_name_expr_debug(const mu_name_expr_t *expr) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Name Expr #%zu: ", expr->as_stator.id);
  mu_name_debug(expr->name);
  putc('\n', stderr);
}
