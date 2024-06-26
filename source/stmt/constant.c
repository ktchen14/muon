#include "constant.h"

#include "../engine.h"
#include "../expr.h"
#include "../name.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

const mu_constant_stmt_t *mu_constant_stmt(
    mu_engine_t *engine, const mu_name_t *name, const mu_expr_t *expr) {
  assert(name->as_stator.engine == engine);
  assert(expr->as_stator.engine == engine);

  size_t size = sizeof(mu_constant_stmt_t);

  mu_constant_stmt_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_constant_stmt_t) {.name = name, .expr = expr};

  return engine_assign_concrete(engine, result);
}

void mu_constant_stmt_debug(const mu_constant_stmt_t *stmt) {
  fprintf(stderr, "Constant Stmt #%zu: ", stmt->as_stator.id);
  mu_name_debug(stmt->name);
  putc('\n', stderr);

  WITH_DEBUG_INDENT() { mu_expr_debug(stmt->expr); }
}
