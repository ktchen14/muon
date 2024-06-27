#ifndef MU_EXPR_ZERO_I
#define MU_EXPR_ZERO_I

#include <muon/expr/zero.h>  // IWYU pragma: export

#include "common.h"
#include "../sign.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline size_t zero_expr_size(const mu_zero_expr_t *expr) {
  return sizeof(mu_zero_expr_t);
}

__attribute__((nonnull))
static inline const mu_sign_t *zero_expr_deduce(
    mu_engine_t *engine,
    const mu_zero_expr_t *expr,
    const mu_sign_t *equation[]) {
  return &mu_variable_sign(engine, &expr->as_node.source)->as_sign;
}

#endif /* MU_EXPR_ZERO_I */
