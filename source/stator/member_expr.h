#ifndef MU_STATOR_MEMBER_EXPR_I
#define MU_STATOR_MEMBER_EXPR_I

#include <muon/stator/member_expr.h>  // IWYU pragma: export

#include "abstract_node.h"
#include "../inductor.h"

#include <assert.h>
#include <stddef.h>

__attribute__((nonnull, pure))
static inline const mu_node_t *member_expr_at(
    const mu_member_expr_t *expr, size_t i) {
  return i == 0 ? &expr->matter->as_node : NULL;
}

__attribute__((nonnull))
static inline inductor_t *member_expr_induce(
    const mu_member_expr_t *expr, inductor_t *inductor) {
  assert(0);
}

#endif /* MU_STATOR_MEMBER_EXPR_I */
