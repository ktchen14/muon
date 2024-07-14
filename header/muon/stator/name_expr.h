#ifndef MU_STATOR_NAME_EXPR_H
#define MU_STATOR_NAME_EXPR_H

#include "abstract_node.h"

#include "name.h"

typedef struct {
  MU_EXPR_HEADER;

  const mu_name_t *name;
} mu_name_expr_t;

const mu_name_expr_t *mu_name_expr(
    mu_engine_t *engine, const mu_name_t *name, const mu_node_source_t *source)
  __attribute__((malloc, nonnull(1, 2)));

void mu_name_expr_debug(const mu_name_expr_t *expr)
  __attribute__((nonnull));

#endif /* MU_STATOR_NAME_EXPR_H */
