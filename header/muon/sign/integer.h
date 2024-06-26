#ifndef MU_SIGN_INTEGER_H
#define MU_SIGN_INTEGER_H

#include "common.h"
#include "../status.h"

typedef struct {
  MU_SIGN_HEADER;
} mu_integer_sign_t;

const mu_integer_sign_t *mu_integer_sign(
    mu_engine_t *engine, const mu_source_t *source)
  __attribute__((malloc, nonnull(1)));

#endif /* MU_SIGN_INTEGER_H */
