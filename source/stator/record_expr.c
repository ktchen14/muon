#include "record_expr.h"

#include "engine.h"
#include "name.h"
#include "node.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/// Emit debugging information on the expr @a member to the debug stream
static void expr_member_debug(mu_expr_member_t member);

const mu_record_expr_t *mu_record_expr(
    mu_engine_t *engine, size_t argc, const mu_expr_member_t argv[argc]) {
  mu_record_expr_t *result;
  if ((result = record_expr_allocate(engine, argc)) == NULL)
    return NULL;
  memcpy(&result->argv, argv, sizeof(const mu_expr_member_t[argc]));
  return record_expr_activate(result);
}

mu_record_expr_t *record_expr_allocate(mu_engine_t *engine, size_t argc) {
  size_t size;
  if (rare((size = struct_size(mu_record_expr_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_record_expr_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_record_expr_t) { .as_stator.engine = engine, .argc = argc };
  return result;
}

const mu_record_expr_t *record_expr_activate(mu_record_expr_t *expr) {
  mu_engine_t *engine = (mu_engine_t *) expr->as_stator.engine;

  for (size_t i = 0; i < expr->argc; i++) {
    const mu_name_t *member_name = expr->argv[i].name;
    const mu_expr_t *member_expr = expr->argv[i].expr;
    assert(member_name == NULL || member_name->as_stator.engine == engine);
    assert(member_expr != NULL);
    assert(member_expr->as_stator.engine == engine);
  }

  mu_record_expr_t source = {
    .as_expr.kind = MU_RECORD_EXPR, .argc = expr->argc
  };
  memcpy(expr, &source, offsetof(mu_record_expr_t, argv));
  return assign_node(engine, expr);
}

#include "debug.h"

void mu_record_expr_debug(const mu_record_expr_t *expr) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Record Expr #%zu:", expr->as_stator.id);
  debug_node_type(&expr->as_node);
  putc('\n', stderr);

  WITH_DEBUG_INDENT() {
    for (size_t i = 0; i < expr->argc; i++)
      expr_member_debug(expr->argv[i]);
  }
}

static void expr_member_debug(mu_expr_member_t member) {
  fprintf(stderr, "%*s", debug_indent, "");
  if (member.name != NULL) {
    fprintf(stderr, "Member: ");
    mu_name_debug(member.name);
    putc('\n', stderr);
  } else
    fputs("Member:\n", stderr);

  WITH_DEBUG_INDENT() { mu_expr_debug(member.expr); }
}
