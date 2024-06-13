#ifndef MU_EXPR_VECTOR_H
#define MU_EXPR_VECTOR_H

#include "common.h"

#include <stddef.h>

typedef struct {
  MU_EXPR_HEADER;

  size_t argc;

  const mu_expr_t *argv[/* argc */];
} mu_vector_expr_t;

const mu_vector_expr_t *mu_vector_expr(
    mu_engine_t *engine, size_t argc, const mu_expr_t *argv[argc])
  __attribute__((nonnull(1)));

#endif /* MU_EXPR_VECTOR_H */
