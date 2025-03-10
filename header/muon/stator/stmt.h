#ifndef MU_STATOR_STMT_H
#define MU_STATOR_STMT_H

#include "common.h"

#include "name.h"

/// The header that each concrete stmt must have
#define MU_STMT_HEADER union { mu_stmt_t as_stmt; mu_node_t as_node; }

typedef struct {
  MU_STMT_HEADER;
  const mu_name_t *name;
  const mu_expr_t *expr;
} mu_define_stmt_t;

typedef struct {
  MU_STMT_HEADER;
  const mu_name_t *name;
  const mu_sign_t *sign;
} mu_type_stmt_t;

const mu_define_stmt_t *mu_define_stmt(
    mu_engine_t *engine, const mu_name_t *name, const mu_expr_t *expr)
  __attribute__((malloc, nonnull));

const mu_type_stmt_t *mu_type_stmt(
    mu_engine_t *engine, const mu_name_t *name, const mu_sign_t *sign)
  __attribute__((malloc, nonnull));

void mu_define_stmt_debug(const mu_define_stmt_t *stmt)
  __attribute__((nonnull));

void mu_type_stmt_debug(const mu_type_stmt_t *stmt)
  __attribute__((nonnull));

#endif /* MU_STATOR_STMT_H */
