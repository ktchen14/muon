#include "vector.h"

#include "../engine.h"
#include "../expr.h"
#include "../menu.h"
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

  return engine_assign_concrete(engine, result);
}

criteria_t *vector_expr_induce(
    const mu_vector_expr_t *expr,
    criteria_t *criteria,
    const induce_menu_t *menu) {
  assert(expr->as_stator.engine == menu->engine);

  // The type of a vector expr is [a]
  const mu_variable_type_t *a;
  if ((a = mu_variable_type(menu->engine)) == NULL)
    return NULL;

  const mu_vector_type_t *type;
  if ((type = mu_vector_type(menu->engine, &a->as_type)) == NULL)
    return NULL;

  // Now we add the constraint that x must be equivalent to the type of each
  // element. First, reallocate the criteria.
  criteria_t *result;
  if (rare((result = criteria_extend(criteria, expr->argc)) == NULL))
    return NULL;

  // Then populate it
  for (size_t i = expr->argc; i-- > 0;) {
    const mu_expr_t *argument = expr->argv[i];
    result->data[result->length - i] = (constraint_t) {
      .a.type = &a->as_type, .b.node = &argument->as_node,
    };
  }

  return result;
}

void mu_vector_expr_debug(const mu_vector_expr_t *expr) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Vector Expr #%zu:\n", expr->as_stator.id);

  WITH_DEBUG_INDENT() {
    for (size_t i = 0; i < expr->argc; i++)
      mu_expr_debug(expr->argv[i]);
  }
}
