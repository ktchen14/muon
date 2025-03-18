#ifndef MU_STATOR_STMT_I
#define MU_STATOR_STMT_I

#include <muon/stator/stmt.h>  // IWYU pragma: export

#include <stddef.h>

__attribute__((nonnull, const))
static inline const mu_node_t *datatype_option_at(
    const mu_datatype_option_t *option, size_t i) {
  return NULL;
}

__attribute__((nonnull, pure))
static inline const mu_node_t *datatype_stmt_at(
    const mu_datatype_stmt_t *stmt, size_t i) {
  return i < stmt->argc ? &stmt->argv[i]->as_node : NULL;
}

__attribute__((nonnull, pure))
static inline const mu_node_t *define_stmt_at(
    const mu_define_stmt_t *stmt, size_t i) {
  return i == 0 ? &stmt->expr->as_node : NULL;
}

mu_datatype_stmt_t *datatype_stmt_allocate(mu_engine_t *engine, size_t argc)
  __attribute__((malloc, nonnull));

const mu_datatype_stmt_t *datatype_stmt_activate(
    mu_datatype_stmt_t *stmt, const mu_name_t *name)
  __attribute__((nonnull));

#endif /* MU_STATOR_STMT_I */
