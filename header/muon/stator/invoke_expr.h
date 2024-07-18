#ifndef MU_STATOR_INVOKE_EXPR_H
#define MU_STATOR_INVOKE_EXPR_H

#include "abstract_node.h"

#include <stddef.h>

typedef struct {
  MU_EXPR_HEADER;

  const mu_expr_t *lambda;
  const mu_expr_t *matter;
} mu_invoke_expr_t;

const mu_invoke_expr_t *mu_invoke_expr(
    mu_engine_t *engine, const mu_expr_t *lambda, const mu_expr_t *matter)
  __attribute__((malloc, nonnull));

void mu_invoke_expr_debug(const mu_invoke_expr_t *expr)
  __attribute__((nonnull));

#endif /* MU_STATOR_INVOKE_EXPR_H */
