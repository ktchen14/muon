#ifndef MU_STATOR_SEQUENCE_EXPR_I
#define MU_STATOR_SEQUENCE_EXPR_I

#include <muon/stator/sequence_expr.h>  // IWYU pragma: export

#include "abstract_node.h"

#include <stddef.h>

/// Return the <em>i</em>th node in the sequence @a expr
__attribute__((nonnull, pure))
static inline const mu_node_t *sequence_expr_at(
    const mu_sequence_expr_t *expr, size_t i) {
  if (i < expr->argc)
    return &expr->argv[i]->as_node;
  if (i == expr->argc)
    return &expr->output->as_node;
  return NULL;
}

#endif /* MU_STATOR_SEQUENCE_EXPR_I */
