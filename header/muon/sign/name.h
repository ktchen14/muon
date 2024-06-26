#ifndef MU_SIGN_NAME_H
#define MU_SIGN_NAME_H

#include "common.h"
#include "../name.h"
#include "../status.h"

typedef struct {
  MU_SIGN_HEADER;

  const mu_name_t *name;
} mu_name_sign_t;

const mu_name_sign_t *mu_name_sign(
    mu_engine_t *engine, const mu_name_t *name, const mu_source_t *source)
  __attribute__((malloc, nonnull(1, 2)));

void mu_name_sign_debug(const mu_name_sign_t *sign)
  __attribute__((nonnull));

#endif /* MU_SIGN_NAME_H */
