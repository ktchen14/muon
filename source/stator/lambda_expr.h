#ifndef MU_STATOR_LAMBDA_EXPR_I
#define MU_STATOR_LAMBDA_EXPR_I

#include <muon/stator/lambda_expr.h>  // IWYU pragma: export

#include "abstract_node.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline const mu_node_t *lambda_expr_at(
    const mu_lambda_expr_t *expr, size_t i) {
  switch (i) {
    case 0: return &expr->argument->as_node;
    case 1: return &expr->matter->as_node;
    default: return NULL;
  }
}

#endif /* MU_STATOR_LAMBDA_EXPR_I */
