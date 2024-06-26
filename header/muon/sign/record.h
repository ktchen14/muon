#ifndef MU_SIGN_RECORD_H
#define MU_SIGN_RECORD_H

#include "common.h"
#include "../status.h"

#include <stddef.h>

typedef struct {
  MU_SIGN_HEADER;

  size_t argc;

  const mu_sign_t *argv[/* argc */];
} mu_record_sign_t;

const mu_record_sign_t *mu_record_sign(
    mu_engine_t *engine,
    size_t argc,
    const mu_sign_t *argv[static argc],
    const mu_source_t *source)
  __attribute__((malloc, nonnull(1)));

#endif /* MU_SIGN_RECORD_H */
