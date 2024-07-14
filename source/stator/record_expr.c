#include "record_expr.h"

#include "engine.h"
#include "node.h"
#include "type.h"
#include "../inductor.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

const mu_record_expr_t *mu_record_expr(
    mu_engine_t *engine, size_t argc, const mu_expr_t *const argv[argc]) {
  for (size_t i = 0; i < argc; i++) {
    assert(argv[i] != NULL);
    assert(argv[i]->as_stator.engine == engine);
  }

  size_t size;
  if (rare((size = struct_size(mu_record_expr_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_record_expr_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_record_expr_t) {
    .as_expr.kind = MU_RECORD_EXPR, .argc = argc,
  };
  memcpy(&result->argv, argv, sizeof(const mu_expr_t *[argc]));

  return assign_node(engine, result);
}

const mu_type_t *record_expr_induce(
    const mu_record_expr_t *expr, inductor_t *inductor) {
  mu_engine_t *engine = inductor->engine;

  mu_record_type_t *allocation;
  if ((allocation = record_type_allocate(engine, expr->argc)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++) {
    const mu_expr_t *argument = expr->argv[i];
    const mu_type_t *type = inductor_node(inductor, &argument->as_node);
    assert(type != NULL);
    allocation->argv[i] = type;
  }

  const mu_record_type_t *record_type = record_type_activate(allocation);
  return &record_type->as_type;
}

void mu_record_expr_debug(const mu_record_expr_t *expr) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Record Expr #%zu:\n", expr->as_stator.id);

  WITH_DEBUG_INDENT() {
    for (size_t i = 0; i < expr->argc; i++)
      mu_expr_debug(expr->argv[i]);
  }
}
