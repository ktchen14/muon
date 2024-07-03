#ifndef MU_NODE_ZERO_EXPR_I
#define MU_NODE_ZERO_EXPR_I

#include <muon/node/zero_expr.h>  // IWYU pragma: export

#include "common.h"
#include "../inductor.h"
#include "../type.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline const mu_node_t *zero_expr_at(
    const mu_zero_expr_t *expr, size_t i) {
  return NULL;
}

__attribute__((nonnull))
static inline inductor_t *zero_expr_induce(
    const mu_zero_expr_t *expr, inductor_t *inductor) {
  return inductor;
}

#endif /* MU_NODE_ZERO_EXPR_I */
