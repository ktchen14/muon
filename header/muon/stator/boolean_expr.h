#ifndef MU_STATOR_BOOLEAN_EXPR_H
#define MU_STATOR_BOOLEAN_EXPR_H

#include "abstract_node.h"

typedef struct {
  MU_EXPR_HEADER;

  _Bool data;
} mu_boolean_expr_t;

const mu_boolean_expr_t *mu_boolean_expr(mu_engine_t *engine, _Bool data)
  __attribute__((malloc, nonnull));

void mu_boolean_expr_debug(const mu_boolean_expr_t *expr)
  __attribute__((nonnull));

#endif /* MU_STATOR_BOOLEAN_EXPR_H */
