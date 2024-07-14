#include "member_expr.h"

#include "engine.h"
#include "name.h"
#include "node.h"
#include "type.h"
#include "../inductor.h"

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
  return assign_node(engine, result);
}

inductor_t *member_expr_induce(
    const mu_member_expr_t *expr, inductor_t *inductor) {
  mu_engine_t *engine = inductor->engine;

  const mu_variable_type_t *matter_type;
  if ((matter_type = mu_variable_type(engine)) == NULL)
    return NULL;

  const mu_node_t *node = &expr->matter->as_node;
  const mu_type_t *type = &matter_type->as_type;
  if ((inductor_equate_node_type(inductor, node, type)) == NULL)
    return NULL;

  const mu_member_type_t *member_type;
  type = &matter_type->as_type;
  if ((member_type = mu_member_type(engine, expr->name, type)) == NULL)
    return NULL;

  type = &member_type->as_type;
  return inductor_equate_node_type(inductor, &expr->as_node, type);
}

void mu_member_expr_debug(const mu_member_expr_t *expr) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Member Expr #%zu: ", expr->as_stator.id);
  mu_name_debug(expr->name);
  putc('\n', stderr);

  WITH_DEBUG_INDENT() { mu_expr_debug(expr->matter); }
}
