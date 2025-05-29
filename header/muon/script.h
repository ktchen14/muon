#ifndef MU_SCRIPT_H
#define MU_SCRIPT_H

#include "engine.h"

#include <stddef.h>

typedef struct {
  size_t argc;
  MuonStmt *argv[/* argc */];
} mu_script_t;

mu_script_t *mu_script(size_t argc, MuonStmt *argv[argc])
  __attribute__((malloc));

MuonSequenceExpr *mu_script_to_sequence_expr(
    MuonEngine *engine, const mu_script_t *script);

MuonSequenceExpr *mu_script_to_sequence_expr_with_prefix(
    MuonEngine *engine,
    const mu_script_t *script,
    size_t length,
    MuonStmt *prefix[]);

void mu_script_debug(const mu_script_t *script) __attribute__((nonnull));

#endif /* MU_SCRIPT_H */
