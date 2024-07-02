#ifndef MU_EXPR_VECTOR_I
#define MU_EXPR_VECTOR_I

#include <muon/expr/vector.h>  // IWYU pragma: export

#include "common.h"
#include "../inductor.h"

#include <stddef.h>

/// Return the size of the vector @a expr
__attribute__((nonnull, pure))
static inline size_t vector_expr_size(const mu_vector_expr_t *expr) {
  return extant_size(mu_vector_expr_t, argv, expr->argc);
}

/// Return the <em>i</em>th node in the vector @a expr
__attribute__((nonnull, pure))
static inline const mu_node_t *vector_expr_at(
    const mu_vector_expr_t *expr, size_t i) {
  return i < expr->argc ? &expr->argv[i]->as_node : NULL;
}

__attribute__((nonnull))
inductor_t *vector_expr_induce(
    const mu_vector_expr_t *expr, inductor_t *inductor);

#endif /* MU_EXPR_VECTOR_I */
