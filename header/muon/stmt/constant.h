#ifndef MU_STMT_VECTOR_H
#define MU_STMT_VECTOR_H

#include "common.h"
#include "../expr/common.h"
#include "../name.h"

#include <stddef.h>

typedef struct {
  MU_STMT_HEADER;

  const mu_name_t *name;
  const mu_expr_t *expr;
} mu_constant_stmt_t;

const mu_constant_stmt_t *mu_constant_stmt(
    mu_engine_t *engine, const mu_name_t *name, const mu_expr_t *expr)
  __attribute__((nonnull));

#endif /* MU_STMT_VECTOR_H */
