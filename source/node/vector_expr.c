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

  return node_assign(engine, result);
}

inductor_t *vector_expr_induce(
    const mu_vector_expr_t *expr, inductor_t *inductor) {
  // The type of a vector expr is [a]
  const mu_variable_type_t *a;
  if ((a = mu_variable_type(inductor->engine)) == NULL)
    return NULL;

  const mu_vector_type_t *type;
  if ((type = mu_vector_type(inductor->engine, &a->as_type)) == NULL)
    return NULL;

  if ((inductor = inductor_extend_type(inductor, &type->as_type)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++) {
    const mu_expr_t *argument = expr->argv[i];
    inductor = inductor_equate_node_type(inductor, &argument->as_node, &a->as_type);
    if (inductor == NULL)
      return NULL;
  }

  return inductor_equate_node_type(inductor, &expr->as_node, &type->as_type);
}

void mu_vector_expr_debug(const mu_vector_expr_t *expr) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Vector Expr #%zu:\n", expr->as_stator.id);

  WITH_DEBUG_INDENT() {
    for (size_t i = 0; i < expr->argc; i++)
      mu_expr_debug(expr->argv[i]);
  }
}
