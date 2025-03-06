#ifndef MU_STATOR_DEFINE_STMT_H
#define MU_STATOR_DEFINE_STMT_H

#include "abstract_node.h"

#include "expr.h"
#include "name.h"

typedef struct {
  MU_STMT_HEADER;

  const mu_name_t *name;
  const mu_expr_t *expr;
  const mu_sign_t *sign;
} mu_define_stmt_t;

const mu_define_stmt_t *mu_define_stmt(
    mu_engine_t *engine,
    const mu_name_t *name,
    const mu_expr_t *expr,
    const mu_sign_t *sign)
  __attribute__((malloc, nonnull(1, 2, 3)));

void mu_define_stmt_debug(const mu_define_stmt_t *stmt)
  __attribute__((nonnull));

#endif /* MU_STATOR_DEFINE_STMT_H */
