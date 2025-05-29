#ifndef MU_SCRIPT_H
#define MU_SCRIPT_H

#include "engine.h"

#include <stddef.h>

typedef struct {
  size_t argc;
  mu_stmt_t *argv[/* argc */];
} mu_script_t;

mu_script_t *mu_script(size_t argc, mu_stmt_t *argv[argc])
  __attribute__((malloc));

mu_sequence_expr_t *mu_script_to_sequence_expr(
    mu_engine_t *engine, const mu_script_t *script);

mu_sequence_expr_t *mu_script_to_sequence_expr_with_prefix(
    mu_engine_t *engine,
    const mu_script_t *script,
    size_t length,
    mu_stmt_t *prefix[]);

void mu_script_debug(const mu_script_t *script) __attribute__((nonnull));

#endif /* MU_SCRIPT_H */
