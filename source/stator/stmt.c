#include "stmt.h"

#include "engine.h"
#include "name.h"
#include "node.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

const mu_define_stmt_t *mu_define_stmt(
    mu_engine_t *engine, const mu_name_t *name, const mu_expr_t *expr) {
  assert(name->engine == engine);
  assert(expr->as_node.engine == engine);

  mu_define_stmt_t *result;
  if ((result = node_allocate(engine, sizeof(mu_define_stmt_t))) == NULL)
    return NULL;
  *result = (mu_define_stmt_t) {
    .as_stmt.kind = MU_DEFINE_STMT, .name = name, .expr = expr,
  };
  return assign_node(engine, result);
}

const mu_datatype_stmt_t *mu_datatype_stmt(
    mu_engine_t *engine, const mu_name_t *name, const mu_sign_t *sign) {
  assert(name->engine == engine);
  assert(sign->as_node.engine == engine);

  mu_datatype_stmt_t *result;
  if ((result = node_allocate(engine, sizeof(mu_datatype_stmt_t))) == NULL)
    return NULL;
  *result = (mu_datatype_stmt_t) {
    /* .as_stmt.kind = MU_TYPE_STMT, .name = name, .sign = sign, */
  };
  return assign_node(engine, result);
}

#include "debug.h"

void mu_datatype_option_debug(const mu_datatype_option_t *option) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, PRIsKIND "#" PRIuID "(name = ",
      DEBUG_KIND("DatatypeOption"), DEBUG_ID(option->as_node.id));
  mu_name_debug(option->name);
  debug_node_type(&option->as_node);
  putc('\n', stderr);
}

void mu_datatype_stmt_debug(const mu_datatype_stmt_t *stmt) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, PRIsKIND "#" PRIuID "(name = ",
      DEBUG_KIND("DatatypeStmt"), DEBUG_ID(stmt->as_node.id));
  mu_name_debug(stmt->name);
  debug_node_type(&stmt->as_node);
  putc('\n', stderr);

  WITH_DEBUG_INDENT() {
    for (size_t i = 0; i < stmt->argc; i++)
      mu_datatype_option_debug(stmt->argv[i]);
  }
}

void mu_define_stmt_debug(const mu_define_stmt_t *stmt) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, PRIsKIND "#" PRIuID "(name = ",
      DEBUG_KIND("DefineStmt"), DEBUG_ID(stmt->as_node.id));
  mu_name_debug(stmt->name);
  putc(')', stderr);
  debug_node_type(&stmt->as_node);
  putc('\n', stderr);

  WITH_DEBUG_INDENT() { mu_expr_debug(stmt->expr); }
}
