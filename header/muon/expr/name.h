#ifndef MU_EXPR_NAME_H
#define MU_EXPR_NAME_H

#include "common.h"
#include "../name.h"

typedef struct {
  MU_EXPR_HEADER;

  const mu_name_t *name;
} mu_name_expr_t;

const mu_name_expr_t *mu_name_expr(
    mu_engine_t *engine, const mu_name_t *name, const mu_source_t *source)
  __attribute__((malloc, nonnull(1, 2)));

void mu_name_expr_debug(const mu_name_expr_t *expr)
  __attribute__((nonnull));

#endif /* MU_EXPR_NAME_H */
