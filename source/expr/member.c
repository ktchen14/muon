#include "member.h"

#include "../common.h"
#include "../engine.h"

#include <assert.h>

const mu_member_expr_t *mu_member_expr(
    mu_engine_t *engine, const mu_name_t *name, const mu_expr_t *matter) {
  assert(name->as_stator.engine == engine);
  assert(matter->as_stator.engine == engine);

  size_t size = sizeof(mu_member_expr_t);

  mu_member_expr_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_member_expr_t) {
    .as_expr.kind = MU_MEMBER_EXPR, .name = name, .matter = matter
  };
  return engine_assign_concrete(engine, result);
}
