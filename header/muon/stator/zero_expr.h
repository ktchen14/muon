#ifndef MU_STATOR_ZERO_EXPR_H
#define MU_STATOR_ZERO_EXPR_H

#include "node.h"

typedef struct {
  MU_EXPR_HEADER;
} mu_zero_expr_t;

const mu_zero_expr_t *mu_zero_expr(mu_engine_t *engine)
  __attribute__((malloc, nonnull));

void mu_zero_expr_debug(const mu_zero_expr_t *expr) __attribute__((nonnull));

#endif /* MU_STATOR_ZERO_EXPR_H */
