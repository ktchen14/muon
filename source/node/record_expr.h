#ifndef MU_EXPR_RECORD_I
#define MU_EXPR_RECORD_I

#include <muon/node/record_expr.h>  // IWYU pragma: export

#include "common.h"
#include "../inductor.h"

#include <assert.h>
#include <stddef.h>

__attribute__((nonnull, pure))
static inline const mu_node_t *record_expr_at(
    const mu_record_expr_t *expr, size_t i) {
  return i < expr->argc ? &expr->argv[i]->as_node : NULL;
}

__attribute__((nonnull))
static inline inductor_t *record_expr_induce(
    const mu_record_expr_t *expr, inductor_t *inductor) {
  assert(0);
}

#endif /* MU_EXPR_RECORD_I */
