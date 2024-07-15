#ifndef MU_STATOR_ACCESS_EXPR_I
#define MU_STATOR_ACCESS_EXPR_I

#include <muon/stator/access_expr.h>  // IWYU pragma: export

#include "abstract_node.h"
#include "abstract_type.h"
#include "member_test.h"
#include "variable_type.h"
#include "../inductor.h"

#include <assert.h>
#include <stddef.h>

__attribute__((nonnull, pure))
static inline const mu_node_t *access_expr_at(
    const mu_access_expr_t *expr, size_t i) {
  return i == 0 ? &expr->matter->as_node : NULL;
}

__attribute__((nonnull))
static inline const mu_type_t *access_expr_induce(
    const mu_access_expr_t *expr, inductor_t *inductor) {
  mu_engine_t *engine = inductor->engine;

  const mu_variable_type_t *open_type;
  if ((open_type = mu_open_type(engine)) == NULL)
    return NULL;

  const mu_member_test_t *member_test;
  if ((member_test = mu_member_test(engine, expr->name, &open_type->as_type)) == NULL)
    return NULL;

  const mu_variable_type_t *variable_type;
  if ((variable_type = mu_variable_type(engine, 1, (const mu_test_t *[]) { &member_test->as_test })) == NULL)
    return NULL;

  const mu_type_t *matter = inductor_node(inductor, &expr->matter->as_node);
  assert(matter != NULL);

  if (inductor_equate(inductor, &variable_type->as_type, matter) == NULL)
    return NULL;

  return &open_type->as_type;
}

#endif /* MU_STATOR_ACCESS_EXPR_I */
