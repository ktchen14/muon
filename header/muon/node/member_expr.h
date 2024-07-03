#ifndef MU_NODE_MEMBER_EXPR_H
#define MU_NODE_MEMBER_EXPR_H

#include "common.h"
#include "../name.h"

typedef struct {
  MU_EXPR_HEADER;

  const mu_name_t *name;
  const mu_expr_t *matter;
} mu_member_expr_t;

const mu_member_expr_t *mu_member_expr(
    mu_engine_t *engine, const mu_name_t *name, const mu_expr_t *matter)
  __attribute__((malloc, nonnull));

void mu_member_expr_debug(const mu_member_expr_t *expr)
  __attribute__((nonnull));

#endif /* MU_NODE_MEMBER_EXPR_H */
