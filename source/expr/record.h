#ifndef MU_EXPR_RECORD_I
#define MU_EXPR_RECORD_I

#include <muon/expr/record.h>  // IWYU pragma: export

#include "common.h"
#include "../menu.h"
#include "../type.h"

#include <assert.h>
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

__attribute__((nonnull))
static inline criteria_t *record_expr_induce(
    const mu_record_expr_t *expr,
    criteria_t *criteria,
    const induce_menu_t *menu) {
  assert(0);
}

#endif /* MU_EXPR_RECORD_I */
