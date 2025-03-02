#include "integer_expr.h"

#include "engine.h"
#include "node.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

const mu_integer_expr_t *mu_integer_expr(mu_engine_t *engine, uint64_t data) {
  size_t size = sizeof(mu_integer_expr_t);

  mu_integer_expr_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_integer_expr_t) {
    .as_expr.kind = MU_INTEGER_EXPR, .data = data,
  };

  return assign_node(engine, result);
}

#include "debug.h"

void mu_integer_expr_debug(const mu_integer_expr_t *expr) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, PRIsKIND "#" PRIuID "(data = %" PRIu64 ")\n",
    DEBUG_KIND("IntegerExpr"), DEBUG_ID(expr->as_stator.id), expr->data);
}
