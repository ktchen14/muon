#ifndef MU_EXPR_VECTOR_I
#define MU_EXPR_VECTOR_I

#include <muon/expr/vector.h>  // IWYU pragma: export

#include "common.h"
#include "../sign.h"

#include <stddef.h>

__attribute__((nonnull, pure))
static inline size_t vector_expr_size(const mu_vector_expr_t *expr) {
  return extant_size(mu_vector_expr_t, argv, expr->argc);
}

const mu_sign_t *vector_expr_induce(
    mu_engine_t *engine,
    const mu_vector_expr_t *expr,
    criteria_t **criteriap,
    const mu_sign_t *const equation[])
  __attribute__((nonnull));

#endif /* MU_EXPR_VECTOR_I */
