#include "stmt.h"

#include "engine.h"
#include "name.h"
#include "node.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

const mu_datatype_option_t *mu_datatype_option(
    mu_engine_t *engine, const mu_name_t *name) {
  assert(name->engine == engine);

  mu_datatype_option_t *result;
  if ((result = node_allocate(engine, sizeof(mu_datatype_option_t))) == NULL)
    return NULL;
  *result = (mu_datatype_option_t) {
    .as_node.kind = MU_DATATYPE_OPTION_NODE, .name = name,
  };
  return assign_node(engine, result);
}

const mu_datatype_stmt_t *mu_datatype_stmt(
    mu_engine_t *engine,
    const mu_name_t *name,
    size_t argc,
    const mu_datatype_option_t *argv[/* argc */]) {
  mu_datatype_stmt_t *result;
  if ((result = datatype_stmt_allocate(engine, argc)) == NULL)
    return NULL;
  for (size_t i = 0; i < argc; i++)
    result->argv[i] = argv[i];
  return datatype_stmt_activate(result, name);
}

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

mu_datatype_stmt_t *datatype_stmt_allocate(mu_engine_t *engine, size_t argc) {
  size_t size;
  if (rare((size = struct_size(mu_datatype_stmt_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_datatype_stmt_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_datatype_stmt_t) { .as_node.engine = engine, .argc = argc };
  return result;
}

const mu_datatype_stmt_t *datatype_stmt_activate(
    mu_datatype_stmt_t *stmt, const mu_name_t *name) {
  mu_engine_t *engine = (mu_engine_t *) stmt->as_node.engine;

  assert(name->engine == engine);

  for (size_t i = 0; i < stmt->argc; i++) {
    assert(stmt->argv[i] != NULL);
    assert(stmt->argv[i]->as_node.engine == engine);
  }

  mu_datatype_stmt_t source = {
    .as_stmt.kind = MU_DATATYPE_STMT, .name = name, .argc = stmt->argc,
  };
  memcpy(stmt, &source, offsetof(mu_datatype_stmt_t, argv));
  return assign_node(engine, stmt);
}

#include "debug.h"

void mu_datatype_option_debug(const mu_datatype_option_t *option) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, PRIsKIND "#" PRIuID "(name = ",
      DEBUG_KIND("DatatypeOption"), DEBUG_ID(option->as_node.id));
  mu_name_debug(option->name);
  putc(')', stderr);
  debug_node_type(&option->as_node);
  putc('\n', stderr);
}

void mu_datatype_stmt_debug(const mu_datatype_stmt_t *stmt) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, PRIsKIND "#" PRIuID "(name = ",
      DEBUG_KIND("DatatypeStmt"), DEBUG_ID(stmt->as_node.id));
  mu_name_debug(stmt->name);
  putc(')', stderr);
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
