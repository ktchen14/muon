#ifndef MU_NODE_ZERO_EXPR_H
#define MU_NODE_ZERO_EXPR_H

#include "common.h"

typedef struct {
  MU_EXPR_HEADER;
} mu_zero_expr_t;

const mu_zero_expr_t *mu_zero_expr(mu_engine_t *engine)
  __attribute__((malloc, nonnull));

void mu_zero_expr_debug(const mu_zero_expr_t *expr) __attribute__((nonnull));

#endif /* MU_NODE_ZERO_EXPR_H */
