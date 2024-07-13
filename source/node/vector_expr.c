#include "vector_expr.h"

#include "../engine.h"
#include "../inductor.h"
#include "../node.h"
#include "../type.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const mu_vector_expr_t *mu_vector_expr(
    mu_engine_t *engine, size_t argc, const mu_expr_t *argv[argc]) {
  for (size_t i = 0; i < argc; i++)
    assert(argv[i]->as_stator.engine == engine);

  size_t size;
  if (rare((size = struct_size(mu_vector_expr_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_vector_expr_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_vector_expr_t) {
    .as_expr.kind = MU_VECTOR_EXPR, .argc = argc,
  };
  memcpy(&result->argv, argv, sizeof(const mu_expr_t *[argc]));

  return assign_node(engine, result);
}

inductor_t *vector_expr_induce(
    const mu_vector_expr_t *expr, inductor_t *inductor) {
  mu_engine_t *engine = inductor->engine;

  const mu_variable_type_t *matter_type;
  if ((matter_type = mu_variable_type(engine)) == NULL)
    return NULL;

  const mu_vector_type_t *vector_type;
  if ((vector_type = mu_vector_type(engine, &matter_type->as_type)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++) {
    const mu_node_t *node = &expr->argv[i]->as_node;
    const mu_type_t *type = &matter_type->as_type;
    if ((inductor = inductor_equate_node_type(inductor, node, type)) == NULL)
      return NULL;
  }

  const mu_type_t *type = &vector_type->as_type;
  return inductor_equate_node_type(inductor, &expr->as_node, type);
}

void mu_vector_expr_debug(const mu_vector_expr_t *expr) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Vector Expr #%zu:\n", expr->as_stator.id);

  WITH_DEBUG_INDENT() {
    for (size_t i = 0; i < expr->argc; i++)
      mu_expr_debug(expr->argv[i]);
  }
}
