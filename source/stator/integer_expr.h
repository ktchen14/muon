#ifndef MU_STATOR_INTEGER_EXPR_I
#define MU_STATOR_INTEGER_EXPR_I

#include <muon/stator/integer_expr.h>  // IWYU pragma: export

#include "abstract_node.h"
#include "abstract_type.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline const mu_node_t *integer_expr_at(
    const mu_integer_expr_t *expr, size_t i) {
  return NULL;
}

#endif /* MU_STATOR_INTEGER_EXPR_I */
