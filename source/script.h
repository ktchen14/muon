#ifndef MU_SCRIPT_I
#define MU_SCRIPT_I

#include <muon/script.h>  // IWYU pragma: export

#include "engine.h"
#include "status.h"

typedef unsigned char mu_char8_t;

mu_script_t *mu_read_script(
    MuonEngine *engine, mu_status_t *status, const mu_char8_t *buffer)
  __attribute__((nonnull));

#endif /* MU_SCRIPT_I */
