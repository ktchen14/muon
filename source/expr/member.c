#include "member.h"

#include "../engine.h"
#include "../expr.h"
#include "../name.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

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

void member_expr_debug(const mu_member_expr_t *expr) {
  fprintf(stderr, "Member Expr #%zu: ", expr->as_stator.id);
  mu_name_debug(expr->name);
  putc('\n', stderr);

  WITH_DEBUG_INDENT()
    mu_expr_debug(expr->matter);
}
