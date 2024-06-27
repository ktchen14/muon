#ifndef MU_SIGN_VARIABLE_H
#define MU_SIGN_VARIABLE_H

#include "common.h"
#include "../status.h"

typedef struct {
  MU_SIGN_HEADER;

  const mu_sign_t *matter;
} mu_variable_sign_t;

const mu_variable_sign_t *mu_variable_sign(
    mu_engine_t *engine, const mu_source_t *source)
  __attribute__((malloc, nonnull(1)));

void mu_variable_sign_debug(const mu_variable_sign_t *sign)
  __attribute__((nonnull));

#endif /* MU_SIGN_VARIABLE_H */
