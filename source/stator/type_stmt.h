#ifndef MU_STATOR_TYPE_STMT_I
#define MU_STATOR_TYPE_STMT_I

#include <muon/stator/type_stmt.h>  // IWYU pragma: export

#include "abstract_node.h"
#include "abstract_type.h"

#include <assert.h>
#include <stddef.h>

__attribute__((nonnull, pure))
static inline const mu_node_t *type_stmt_at(
    const mu_type_stmt_t *stmt, size_t i) {
  return i == 0 ? &stmt->sign->as_node : NULL;
}

#endif /* MU_STATOR_TYPE_STMT_I */
