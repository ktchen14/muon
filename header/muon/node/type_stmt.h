#ifndef MU_NODE_TYPE_STMT_H
#define MU_NODE_TYPE_STMT_H

#include "common.h"
#include "../name.h"

typedef struct {
  MU_STMT_HEADER;

  const mu_name_t *name;
  const mu_sign_t *sign;
} mu_type_stmt_t;

const mu_type_stmt_t *mu_type_stmt(
    mu_engine_t *engine, const mu_name_t *name, const mu_sign_t *sign)
  __attribute__((malloc, nonnull(1, 2, 3)));

void mu_type_stmt_debug(const mu_type_stmt_t *stmt)
  __attribute__((nonnull));

#endif /* MU_NODE_TYPE_STMT_H */
