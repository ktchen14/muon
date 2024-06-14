#ifndef MU_EXPR_ZERO_H
#define MU_EXPR_ZERO_H

#include "common.h"

typedef struct {
  MU_EXPR_HEADER;
} mu_zero_expr_t;

const mu_zero_expr_t *mu_zero_expr(mu_engine_t *engine)
  __attribute__((nonnull));

#endif /* MU_EXPR_ZERO_H */
