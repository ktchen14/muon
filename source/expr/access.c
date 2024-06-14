#include "access.h"

#include "../common.h"
#include "../engine.h"

#include <assert.h>
#include <string.h>

const mu_access_expr_t *mu_access_expr(
    mu_engine_t *engine, const mu_name_t *name) {
  assert(name->as_stator.engine == engine);

  size_t size = sizeof(mu_access_expr_t);

  mu_access_expr_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_access_expr_t) {
    .as_expr.kind = MU_MEMBER_EXPR, .name = name,
  };
  return engine_assign_concrete(engine, result);
}
