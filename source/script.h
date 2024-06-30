#ifndef MU_SCRIPT_I
#define MU_SCRIPT_I

#include <muon/script.h>  // IWYU pragma: export

#include "common.h"
#include "engine.h"
#include "status.h"

void scan_debug(const unsigned char *string, const char *name)
  __attribute__((nonnull(1)));

mu_script_t *mu_read_script(
    mu_engine_t *engine, mu_status_t *status, const char8_t *buffer)
  __attribute__((nonnull));

#endif /* MU_SCRIPT_I */
