#ifndef MU_STATOR_VECTOR_EXPR_I
#define MU_STATOR_VECTOR_EXPR_I

#include <muon/stator/vector_expr.h>  // IWYU pragma: export

#include "abstract_node.h"
#include "abstract_type.h"

#include "../inductor.h"

#include <stddef.h>

/// Return the <em>i</em>th node in the vector @a expr
__attribute__((nonnull, pure))
static inline const mu_node_t *vector_expr_at(
    const mu_vector_expr_t *expr, size_t i) {
  return i < expr->argc ? &expr->argv[i]->as_node : NULL;
}

const mu_type_t *vector_expr_induce(
    const mu_vector_expr_t *expr, inductor_t *inductor)
  __attribute__((nonnull));

#endif /* MU_STATOR_VECTOR_EXPR_I */
