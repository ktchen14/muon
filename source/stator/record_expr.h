#ifndef MU_STATOR_RECORD_EXPR_I
#define MU_STATOR_RECORD_EXPR_I

#include <muon/stator/record_expr.h>  // IWYU pragma: export

#include "abstract_node.h"

#include <stddef.h>

mu_record_expr_t *record_expr_allocate(mu_engine_t *engine, size_t argc)
  __attribute__((malloc, nonnull));

const mu_record_expr_t *record_expr_activate(mu_record_expr_t *expr)
  __attribute__((nonnull, returns_nonnull));

__attribute__((nonnull, pure))
static inline const mu_node_t *record_expr_at(
    const mu_record_expr_t *expr, size_t i) {
  return i < expr->argc ? &expr->argv[i].expr->as_node : NULL;
}

#endif /* MU_STATOR_RECORD_EXPR_I */
