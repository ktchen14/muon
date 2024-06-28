#include "expr.h"

#include <stdlib.h>

const mu_sign_t *expr_induce(
    mu_engine_t *engine,
    const mu_expr_t *expr,
    criteria_t **criteriap,
    const mu_sign_t *const equation[]) {
  switch (expr->kind) {
    case MU_ACCESS_EXPR:
      abort();

    case MU_INTEGER_EXPR:
      return integer_expr_induce(
          engine, (const mu_integer_expr_t *) expr, criteriap, equation);

    case MU_MEMBER_EXPR:
      abort();

    case MU_NAME_EXPR:
      abort();

    case MU_RECORD_EXPR:
      abort();

    case MU_VECTOR_EXPR:
      return vector_expr_induce(
          engine, (const mu_vector_expr_t *) expr, criteriap, equation);

    case MU_ZERO_EXPR:
      abort();
  }

  return NULL;  // TODO: unreachable
}

void mu_expr_debug(const mu_expr_t *expr) {
  switch (expr->kind) { MU_EACH_EXPR_KIND(MU_ABSTRACT_EXPR_CALL, mu, debug) }
}
