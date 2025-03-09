#ifndef MU_STATOR_STMT_I
#define MU_STATOR_STMT_I

#include <muon/stator/stmt.h>  // IWYU pragma: export

#include <stddef.h>

__attribute__((nonnull, pure))
static inline const mu_node_t *define_stmt_at(
    const mu_define_stmt_t *stmt, size_t i) {
  return i == 0 ? &stmt->expr->as_node : NULL;
}

__attribute__((nonnull, pure))
static inline const mu_node_t *type_stmt_at(
    const mu_type_stmt_t *stmt, size_t i) {
  return i == 0 ? &stmt->sign->as_node : NULL;
}

#endif /* MU_STATOR_STMT_I */
