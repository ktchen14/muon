#ifndef MU_EXPR_RECORD_H
#define MU_EXPR_RECORD_H

#include "common.h"

#include <stddef.h>

typedef struct {
  MU_EXPR_HEADER;

  size_t argc;

  const mu_expr_t *argv[/* argc */];
} mu_record_expr_t;

const mu_record_expr_t *mu_record_expr(
    mu_engine_t *engine, size_t argc, const mu_expr_t *argv[static argc])
  __attribute__((malloc, nonnull(1)));

void mu_record_expr_debug(const mu_record_expr_t *expr)
  __attribute__((nonnull));

#endif /* MU_EXPR_RECORD_H */
