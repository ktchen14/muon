#ifndef MU_STATOR_RECORD_EXPR_H
#define MU_STATOR_RECORD_EXPR_H

#include "abstract_node.h"

#include <stddef.h>

typedef struct {
  MU_EXPR_HEADER;

  size_t argc;

  const mu_expr_t *argv[/* argc */];
} mu_record_expr_t;

const mu_record_expr_t *mu_record_expr(
    mu_engine_t *engine, size_t argc, const mu_expr_t *argv[argc])
  __attribute__((malloc, nonnull(1)));

void mu_record_expr_debug(const mu_record_expr_t *expr)
  __attribute__((nonnull));

#endif /* MU_STATOR_RECORD_EXPR_H */
