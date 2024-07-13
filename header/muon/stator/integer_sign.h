#ifndef MU_STATOR_INTEGER_SIGN_H
#define MU_STATOR_INTEGER_SIGN_H

#include "node.h"

#include "../status.h"

typedef struct {
  MU_SIGN_HEADER;
} mu_integer_sign_t;

const mu_integer_sign_t *mu_integer_sign(
    mu_engine_t *engine, const mu_source_t *source)
  __attribute__((malloc, nonnull(1)));

void mu_integer_sign_debug(const mu_integer_sign_t *sign)
  __attribute__((nonnull));

#endif /* MU_STATOR_INTEGER_SIGN_H */
