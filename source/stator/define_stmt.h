#ifndef MU_STATOR_DEFINE_STMT_I
#define MU_STATOR_DEFINE_STMT_I

#include <muon/stator/define_stmt.h>  // IWYU pragma: export

#include "abstract_node.h"
#include "abstract_type.h"
#include "../inductor.h"

#include <assert.h>
#include <stddef.h>

__attribute__((nonnull, pure))
static inline const mu_node_t *define_stmt_at(
    const mu_define_stmt_t *stmt, size_t i) {
  return i == 0 ? &stmt->expr->as_node : NULL;
}

__attribute__((nonnull, pure))
static inline const mu_type_t *define_stmt_induce(
    const mu_define_stmt_t *stmt, inductor_t *inductor) {
  const mu_type_t *type = inductor_node_type(inductor, &stmt->expr->as_node);
  assert(type != NULL);
  return type;
}

#endif /* MU_STATOR_DEFINE_STMT_I */
