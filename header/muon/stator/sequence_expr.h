#ifndef MU_STATOR_SEQUENCE_EXPR_H
#define MU_STATOR_SEQUENCE_EXPR_H

#include "abstract_node.h"

#include <stddef.h>

typedef struct {
  MU_EXPR_HEADER;

  const mu_expr_t *output;

  size_t argc;
  const mu_stmt_t *argv[/* argc */];
} mu_sequence_expr_t;

const mu_sequence_expr_t *mu_sequence_expr(
    mu_engine_t *engine,
    const mu_expr_t *output,
    size_t argc,
    const mu_stmt_t *const argv[argc])
  __attribute__((malloc, nonnull(1, 2)));

void mu_sequence_expr_debug(const mu_sequence_expr_t *expr)
  __attribute__((nonnull));

#endif /* MU_STATOR_SEQUENCE_EXPR_H */
