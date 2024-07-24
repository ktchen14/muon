#include "sequence_expr.h"

#include "engine.h"
#include "node.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const mu_sequence_expr_t *mu_sequence_expr(
    mu_engine_t *engine,
    const mu_expr_t *output,
    size_t argc,
    const mu_stmt_t *const argv[argc]) {
  assert(output->as_stator.engine == engine);
  for (size_t i = 0; i < argc; i++)
    assert(argv[i]->as_stator.engine == engine);

  size_t size;
  if (rare((size = struct_size(mu_sequence_expr_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_sequence_expr_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_sequence_expr_t) {
    .as_expr.kind = MU_SEQUENCE_EXPR, .output = output, .argc = argc,
  };
  memcpy(&result->argv, argv, sizeof(const mu_expr_t *[argc]));

  return assign_node(engine, result);
}

#include "debug.h"

void mu_sequence_expr_debug(const mu_sequence_expr_t *expr) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Sequence Expr #%zu:", expr->as_stator.id);
  debug_node_type(&expr->as_node);
  putc('\n', stderr);

  WITH_DEBUG_INDENT() {
    mu_expr_debug(expr->output);

    for (size_t i = 0; i < expr->argc; i++)
      mu_stmt_debug(expr->argv[i]);
  }
}
