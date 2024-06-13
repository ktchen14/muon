#include "../expr.h"
#include "../name.h"

const mu_name_t *handle_name(mu_engine_t *engine) {
  const mu_name_t *name;
  name = mu_name(engine, 1, (const mu_char8_t *) "a");
  return name;
}

const mu_expr_t *handle_expr(mu_engine_t *engine) {
  const mu_integer_expr_t *integer_expr;
  integer_expr = mu_integer_expr(engine, 1);
  return &integer_expr->as_expr;
}

const mu_stmt_t *handle_stmt(mu_engine_t *engine) {
}
