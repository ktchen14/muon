#ifndef MU_EXPR_RECORD_I
#define MU_EXPR_RECORD_I

#include <muon/expr/record.h>  // IWYU pragma: export

#include "common.h"

#include <stddef.h>

__attribute__((nonnull, pure))
static inline size_t record_expr_size(const mu_record_expr_t *expr) {
  return extant_size(mu_record_expr_t, argv, expr->argc);
}

__attribute__((nonnull, pure))
static inline const mu_node_t *record_expr_at(
    const mu_record_expr_t *expr, size_t i) {
  return i < expr->argc ? &expr->argv[i]->as_node : NULL;
}

#endif /* MU_EXPR_RECORD_I */
