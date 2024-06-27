#include "vector.h"

#include "../engine.h"
#include "../expr.h"
#include "../sign.h"

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

const mu_sign_t *vector_expr_induce(
    mu_engine_t *engine,
    const mu_vector_expr_t *expr,
    criteria_t **criteriap,
    const mu_sign_t *const equation[]) {
  const mu_source_t *source = &expr->as_node.source;

  // The type of a vector expr like [a, b, c, d] is [x]. Make the variable k:
  const mu_variable_sign_t *x;
  if ((x = mu_variable_sign(engine, source)) == NULL)
    return NULL;

  // Make the type [k]
  const mu_vector_sign_t *vector_sign;
  if ((vector_sign = mu_vector_sign(engine, &x->as_sign, source)) == NULL)
    return NULL;

  // Now we add the constraint that k must be equivalent to the type of each
  // element. First, reallocate the criteria.
  criteria_t *criteria;
  if (rare((criteria = criteria_extend(*criteriap, expr->argc)) == NULL))
    return NULL;

  // Then populate it
  for (size_t i = expr->argc; i-- > 0;) {
    const mu_expr_t *argument = expr->argv[i];
    criteria->data[criteria->length - i] = (constraint_t) {
      .a = &x->as_sign, .b = equation[argument->as_stator.id],
    };
  }

  *criteriap = criteria;
  return &vector_sign->as_sign;
}

void mu_vector_expr_debug(const mu_vector_expr_t *expr) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Vector Expr #%zu:\n", expr->as_stator.id);

  WITH_DEBUG_INDENT() {
    for (size_t i = 0; i < expr->argc; i++)
      mu_expr_debug(expr->argv[i]);
  }
}
