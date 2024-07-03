#ifndef MU_STMT_CONSTANT_I
#define MU_STMT_CONSTANT_I

#include <muon/node/constant_stmt.h>  // IWYU pragma: export

#include "common.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline size_t constant_stmt_size(const mu_constant_stmt_t *stmt) {
  return sizeof(mu_constant_stmt_t);
}

__attribute__((nonnull, pure))
static inline const mu_node_t *constant_stmt_at(
    const mu_constant_stmt_t *stmt, size_t i) {
  return i == 0 ? &stmt->expr->as_node : NULL;
}

typedef struct inductor_t inductor_t;

inductor_t *constant_stmt_induce(
    const mu_constant_stmt_t *stmt, inductor_t *inductor)
  __attribute__((nonnull));

#endif /* MU_STMT_CONSTANT_I */
