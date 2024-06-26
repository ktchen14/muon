#ifndef MU_EXPR_ACCESS_H
#define MU_EXPR_ACCESS_H

#include "common.h"
#include "../name.h"

typedef struct {
  MU_EXPR_HEADER;

  const mu_name_t *name;
  const mu_expr_t *matter;
} mu_access_expr_t;

const mu_access_expr_t *mu_access_expr(
    mu_engine_t *engine,
    const mu_name_t *name,
    const mu_expr_t *matter,
    const mu_source_t *source)
  __attribute__((malloc, nonnull(1, 2, 3)));

void access_expr_debug(const mu_access_expr_t *expr) __attribute__((nonnull));

#endif /* MU_EXPR_ACCESS_H */
