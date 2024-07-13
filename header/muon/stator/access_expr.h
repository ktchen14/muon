#ifndef MU_STATOR_ACCESS_EXPR_H
#define MU_STATOR_ACCESS_EXPR_H

#include "node.h"

#include "name.h"

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

void mu_access_expr_debug(const mu_access_expr_t *expr)
  __attribute__((nonnull));

#endif /* MU_STATOR_ACCESS_EXPR_H */
