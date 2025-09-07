#ifndef MU_SCRIPT_H
#define MU_SCRIPT_H

#include "engine.h"
#include "status.h"

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

typedef unsigned char mu_char8_t;

mu_script_t *muon_scan(
    MuonEngine *engine, mu_status_t *status, const mu_char8_t *buffer)
  __attribute__((nonnull));

void mu_script_debug(const mu_script_t *script) __attribute__((nonnull));

#endif /* MU_SCRIPT_H */
