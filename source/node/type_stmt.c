#include "type_stmt.h"

#include "../engine.h"
#include "../name.h"
#include "../node.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

const mu_type_stmt_t *mu_type_stmt(
    mu_engine_t *engine, const mu_name_t *name, const mu_sign_t *sign) {
  assert(name->as_stator.engine == engine);
  assert(sign->as_stator.engine == engine);

  size_t size = sizeof(mu_type_stmt_t);

  mu_type_stmt_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_type_stmt_t) {
    .as_stmt.kind = MU_TYPE_STMT, .name = name, .sign = sign,
  };

  return node_assign(engine, result);
}

void mu_type_stmt_debug(const mu_type_stmt_t *stmt) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Type Stmt #%zu: ", stmt->as_stator.id);
  mu_name_debug(stmt->name);
  putc('\n', stderr);

  WITH_DEBUG_INDENT() { mu_sign_debug(stmt->sign); }
}
