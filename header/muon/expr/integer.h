#ifndef MU_EXPR_INTEGER_H
#define MU_EXPR_INTEGER_H

#include "common.h"
#include "../status.h"

#include <stdint.h>

typedef struct {
  MU_EXPR_HEADER;

  uint64_t data;
} mu_integer_expr_t;

const mu_integer_expr_t *mu_integer_expr(
    mu_engine_t *engine, uint64_t data, const mu_source_t *source)
  __attribute__((malloc, nonnull(1)));

#endif /* MU_EXPR_INTEGER_H */
