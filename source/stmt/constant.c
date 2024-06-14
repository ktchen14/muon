#include "constant.h"

#include "../common.h"
#include "../engine.h"

#include <string.h>

const mu_constant_stmt_t *mu_constant_stmt(
    mu_engine_t *engine, const mu_name_t *name, const mu_expr_t *expr) {
  size_t size = sizeof(mu_constant_stmt_t);

  mu_constant_stmt_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_constant_stmt_t) {.name = name, .expr = expr};

  return engine_assign_concrete(engine, result);
}
