#ifndef MU_STATOR_INTEGER_EXPR_H
#define MU_STATOR_INTEGER_EXPR_H

#include "node.h"

#include "../status.h"

#include <stdint.h>

typedef struct {
  MU_EXPR_HEADER;

  uint64_t data;
} mu_integer_expr_t;

const mu_integer_expr_t *mu_integer_expr(
    mu_engine_t *engine, uint64_t data, const mu_source_t *source)
  __attribute__((malloc, nonnull(1)));

void mu_integer_expr_debug(const mu_integer_expr_t *expr)
  __attribute__((nonnull));

#endif /* MU_STATOR_INTEGER_EXPR_H */
