#ifndef MU_EXPR_ACCESS_H
#define MU_EXPR_ACCESS_H

#include "common.h"
#include "../name.h"

typedef struct {
  MU_EXPR_HEADER;

  const mu_name_t *name;
} mu_access_expr_t;

const mu_access_expr_t *mu_access_expr(
    mu_engine_t *engine, const mu_name_t *name)
  __attribute__((malloc, nonnull));

#endif /* MU_EXPR_ACCESS_H */
