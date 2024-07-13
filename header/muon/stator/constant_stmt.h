#ifndef MU_STATOR_CONSTANT_STMT_H
#define MU_STATOR_CONSTANT_STMT_H

#include "node.h"

#include "name.h"

typedef struct {
  MU_STMT_HEADER;

  const mu_name_t *name;
  const mu_expr_t *expr;
  const mu_sign_t *sign;
} mu_constant_stmt_t;

const mu_constant_stmt_t *mu_constant_stmt(
    mu_engine_t *engine,
    const mu_name_t *name,
    const mu_expr_t *expr,
    const mu_sign_t *sign)
  __attribute__((malloc, nonnull(1, 2, 3)));

void mu_constant_stmt_debug(const mu_constant_stmt_t *stmt)
  __attribute__((nonnull));

#endif /* MU_STATOR_CONSTANT_STMT_H */
