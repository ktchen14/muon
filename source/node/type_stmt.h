#ifndef MU_STMT_TYPE_I
#define MU_STMT_TYPE_I

#include <muon/node/type_stmt.h>  // IWYU pragma: export

#include "common.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline size_t type_stmt_size(const mu_type_stmt_t *stmt) {
  return sizeof(mu_type_stmt_t);
}

__attribute__((nonnull, pure))
static inline const mu_node_t *type_stmt_at(
    const mu_type_stmt_t *stmt, size_t i) {
  return i == 0 ? &stmt->sign->as_node : NULL;
}

__attribute__((used))
static const mu_sign_t *type_stmt_induce(const mu_type_stmt_t *stmt) {
  return stmt->sign;
}

#endif /* MU_STMT_TYPE_I */
