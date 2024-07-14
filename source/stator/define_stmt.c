#include "define_stmt.h"

#include "engine.h"
#include "name.h"
#include "node.h"
#include "../inductor.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

const mu_define_stmt_t *mu_define_stmt(
    mu_engine_t *engine,
    const mu_name_t *name,
    const mu_expr_t *expr,
    const mu_sign_t *sign) {
  assert(name->as_stator.engine == engine);
  assert(expr->as_stator.engine == engine);
  assert(sign == NULL || sign->as_stator.engine == engine);

  size_t size = sizeof(mu_define_stmt_t);

  mu_define_stmt_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_define_stmt_t) {
    .as_stmt.kind = MU_DEFINE_STMT,
    .name = name,
    .expr = expr,
    .sign = sign,
  };

  return assign_node(engine, result);
}

inductor_t *define_stmt_induce(
    const mu_define_stmt_t *stmt, inductor_t *inductor) {
  return inductor_equate_node_node(inductor, &stmt->as_node, &stmt->expr->as_node);
}

void mu_define_stmt_debug(const mu_define_stmt_t *stmt) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Define Stmt #%zu: name = ", stmt->as_stator.id);
  mu_name_debug(stmt->name);
  putc('\n', stderr);

  WITH_DEBUG_INDENT() {
    mu_expr_debug(stmt->expr);
    if (stmt->sign != NULL)
      mu_sign_debug(stmt->sign);
  }
}
