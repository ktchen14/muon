#ifndef MU_STATOR_MEMBER_EXPR_I
#define MU_STATOR_MEMBER_EXPR_I

#include <muon/stator/member_expr.h>  // IWYU pragma: export

#include "abstract_node.h"
#include "abstract_type.h"
#include "member_type.h"
#include "../inductor.h"

#include <assert.h>
#include <stddef.h>

__attribute__((nonnull, pure))
static inline const mu_node_t *member_expr_at(
    const mu_member_expr_t *expr, size_t i) {
  return i == 0 ? &expr->matter->as_node : NULL;
}

__attribute__((nonnull))
static inline const mu_type_t *member_expr_induce(
    const mu_member_expr_t *expr, inductor_t *inductor) {
  mu_engine_t *engine = inductor->engine;

  const mu_type_t *matter = inductor_node_type(inductor, &expr->matter->as_node);
  assert(matter != NULL);

  const mu_member_type_t *member_type;
  if ((member_type = mu_member_type(engine, expr->name, matter)) == NULL)
    return NULL;
  return &member_type->as_type;
}


#endif /* MU_STATOR_MEMBER_EXPR_I */
