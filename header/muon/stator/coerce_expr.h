#ifndef MU_STATOR_COERCE_EXPR_H
#define MU_STATOR_COERCE_EXPR_H

#include "abstract_node.h"

typedef struct {
  MU_EXPR_HEADER;

  const mu_expr_t *matter;
} mu_coerce_expr_t;

const mu_coerce_expr_t *mu_coerce_expr(
    mu_engine_t *engine, const mu_expr_t *matter)
  __attribute__((malloc, nonnull));

void mu_coerce_expr_debug(const mu_coerce_expr_t *expr)
  __attribute__((nonnull));

#endif /* MU_STATOR_COERCE_EXPR_H */
