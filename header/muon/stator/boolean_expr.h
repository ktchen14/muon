#ifndef MU_STATOR_BOOLEAN_EXPR_H
#define MU_STATOR_BOOLEAN_EXPR_H

#include "abstract_node.h"

typedef struct {
  MU_EXPR_HEADER;

  _Bool data;
} mu_boolean_expr_t;

const mu_boolean_expr_t *mu_boolean_expr(
    mu_engine_t *engine, _Bool data, const mu_node_source_t *source)
  __attribute__((malloc, nonnull(1)));

void mu_boolean_expr_debug(const mu_boolean_expr_t *expr)
  __attribute__((nonnull));

#endif /* MU_STATOR_BOOLEAN_EXPR_H */
