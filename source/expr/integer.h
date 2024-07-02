#ifndef MU_EXPR_INTEGER_I
#define MU_EXPR_INTEGER_I

#include <muon/expr/integer.h>  // IWYU pragma: export

#include "common.h"
#include "../inductor.h"
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
static inline inductor_t *integer_expr_induce(
    const mu_integer_expr_t *expr, inductor_t *inductor) {
  assert(expr->as_stator.engine == inductor->engine);

  const mu_integer_type_t *type = mu_integer_type(inductor->engine);
  return inductor_equate_node_type(inductor, &expr->as_node, &type->as_type);
}

#endif /* MU_EXPR_INTEGER_I */
