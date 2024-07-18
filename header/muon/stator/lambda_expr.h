#ifndef MU_STATOR_LAMBDA_EXPR_H
#define MU_STATOR_LAMBDA_EXPR_H

#include "abstract_node.h"
#include "variable_view.h"

typedef struct {
  MU_EXPR_HEADER;

  const mu_variable_view_t *argument;
  const mu_expr_t *matter;
} mu_lambda_expr_t;

const mu_lambda_expr_t *mu_lambda_expr(
    mu_engine_t *engine, const mu_variable_view_t *argument, const mu_expr_t *matter)
  __attribute__((malloc, nonnull));

void mu_lambda_expr_debug(const mu_lambda_expr_t *expr)
  __attribute__((nonnull));

#endif /* MU_STATOR_LAMBDA_EXPR_H */
