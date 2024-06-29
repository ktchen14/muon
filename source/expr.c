#include "expr.h"

#include <assert.h>

const criteria_t *expr_induce(
    const mu_expr_t *expr,
    criteria_t *criteria,
    const induce_menu_t *menu) {
  switch (expr->kind) {
#define MU_EMIT(lower, upper, _) \
    case MU_##upper##_EXPR: \
      return lower##_expr_induce((const mu_##lower##_expr_t *) expr, criteria, menu);
    MU_EACH_EXPR_KIND(MU_EMIT)
#undef MU_EMIT
  }

  assert(0);
  __builtin_unreachable();
}

void mu_expr_debug(const mu_expr_t *expr) {
  switch (expr->kind) { MU_EACH_EXPR_KIND(MU_ABSTRACT_EXPR_CALL, mu, debug) }
}
