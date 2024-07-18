#include "access_expr.h"

#include "engine.h"
#include "name.h"
#include "node.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

const mu_access_expr_t *mu_access_expr(
    mu_engine_t *engine,
    const mu_name_t *name,
    const mu_expr_t *matter,
    const mu_node_source_t *source) {
  assert(name->as_stator.engine == engine);
  assert(matter->as_stator.engine == engine);

  size_t size = sizeof(mu_access_expr_t);

  mu_access_expr_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_access_expr_t) {
    .as_expr.kind = MU_ACCESS_EXPR, .name = name, .matter = matter,
  };

  if (source != NULL)
    result->as_node.source = *source;

  return assign_node(engine, result);
}

void mu_access_expr_debug(const mu_access_expr_t *expr) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Access Expr #%zu: name = ", expr->as_stator.id);
  mu_name_debug(expr->name);
  putc('\n', stderr);

  WITH_DEBUG_INDENT() { mu_expr_debug(expr->matter); }
}
