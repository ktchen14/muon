#ifndef MU_EXPR_INTEGER_H
#define MU_EXPR_INTEGER_H

#include "common.h"

#include <stdint.h>

typedef struct {
  MU_EXPR_HEADER;

  uint64_t data;
} mu_integer_expr_t;

const mu_integer_expr_t *mu_integer_expr(mu_engine_t *engine, uint64_t data)
  __attribute__((malloc, nonnull));

#endif /* MU_EXPR_INTEGER_H */
