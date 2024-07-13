#ifndef MU_ANALYZER_I
#define MU_ANALYZER_I

#include "common.h"
#include "script.h"
#include "stator.h"

const mu_stmt_t *const *resolve_names(
    mu_engine_t *engine, const mu_script_t *script);

#endif /* MU_ANALYZER_I */
