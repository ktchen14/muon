#include "stmt.h"

#include "engine.h"
#include "name.h"
#include "node.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

const mu_define_stmt_t *mu_define_stmt(
    mu_engine_t *engine,
    const mu_name_t *name,
    const mu_expr_t *expr,
    const mu_sign_t *sign) {
  assert(name->engine == engine);
  assert(expr->as_stator.engine == engine);
  assert(sign == NULL || sign->as_stator.engine == engine);

  mu_define_stmt_t *result;
  if ((result = node_allocate(engine, sizeof(mu_define_stmt_t))) == NULL)
    return NULL;
  *result = (mu_define_stmt_t) {
    .as_stmt.kind = MU_DEFINE_STMT, .name = name, .expr = expr, .sign = sign,
  };
  return assign_node(engine, result);
}

const mu_type_stmt_t *mu_type_stmt(
    mu_engine_t *engine, const mu_name_t *name, const mu_sign_t *sign) {
  assert(name->engine == engine);
  assert(sign->as_stator.engine == engine);

  mu_type_stmt_t *result;
  if ((result = node_allocate(engine, sizeof(mu_type_stmt_t))) == NULL)
    return NULL;
  *result = (mu_type_stmt_t) {
    .as_stmt.kind = MU_TYPE_STMT, .name = name, .sign = sign,
  };
  return assign_node(engine, result);
}

#include "debug.h"

void mu_define_stmt_debug(const mu_define_stmt_t *stmt) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, PRIsKIND "#" PRIuID "(name = ",
      DEBUG_KIND("DefineStmt"), DEBUG_ID(stmt->as_stator.id));
  mu_name_debug(stmt->name);
  putc(')', stderr);
  debug_node_type(&stmt->as_node);
  putc('\n', stderr);

  WITH_DEBUG_INDENT() {
    mu_expr_debug(stmt->expr);
    if (stmt->sign != NULL)
      mu_sign_debug(stmt->sign);
  }
}

void mu_type_stmt_debug(const mu_type_stmt_t *stmt) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Type Stmt #%zu: ", stmt->as_stator.id);
  mu_name_debug(stmt->name);
  debug_node_type(&stmt->as_node);
  putc('\n', stderr);

  WITH_DEBUG_INDENT() { mu_sign_debug(stmt->sign); }
}
