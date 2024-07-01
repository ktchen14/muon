#ifndef MU_EXPR_ZERO_I
#define MU_EXPR_ZERO_I

#include <muon/expr/zero.h>  // IWYU pragma: export

#include "common.h"
#include "../menu.h"
#include "../type.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline size_t zero_expr_size(const mu_zero_expr_t *expr) {
  return sizeof(mu_zero_expr_t);
}

__attribute__((const, nonnull))
static inline const mu_node_t *zero_expr_at(
    const mu_zero_expr_t *expr, size_t i) {
  return NULL;
}

__attribute__((nonnull))
static inline criteria_t *zero_expr_induce(
    const mu_zero_expr_t *expr,
    criteria_t *criteria,
    const induce_t *menu) {
  return criteria;
}

#endif /* MU_EXPR_ZERO_I */
