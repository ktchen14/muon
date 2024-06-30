#ifndef MU_EXPR_INTEGER_I
#define MU_EXPR_INTEGER_I

#include <muon/expr/integer.h>  // IWYU pragma: export

#include "common.h"
#include "../menu.h"
#include "../type.h"

#include <assert.h>
#include <stddef.h>

__attribute__((const, nonnull))
static inline size_t integer_expr_size(const mu_integer_expr_t *expr) {
  return sizeof(mu_integer_expr_t);
}

__attribute__((const, nonnull))
static inline const mu_node_t *integer_expr_at(
    const mu_integer_expr_t *expr, size_t i) {
  return NULL;
}

__attribute__((nonnull))
static inline criteria_t *integer_expr_induce(
    const mu_integer_expr_t *expr,
    criteria_t *criteria,
    const induce_menu_t *menu) {
  assert(expr->as_stator.engine == menu->engine);

  const mu_integer_type_t *type = mu_integer_type(menu->engine);
  constraint_t constraint = {
    .a.node = &expr->as_node, .b.type = &type->as_type,
  };
  return criteria_append(criteria, constraint);
}

#endif /* MU_EXPR_INTEGER_I */
