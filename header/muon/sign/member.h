#ifndef MU_SIGN_MEMBER_H
#define MU_SIGN_MEMBER_H

#include "common.h"
#include "../name.h"
#include "../status.h"

typedef struct {
  MU_SIGN_HEADER;

  const mu_name_t *name;
  const mu_sign_t *matter;
} mu_member_sign_t;

const mu_member_sign_t *mu_member_sign(
    mu_engine_t *engine,
    const mu_name_t *name,
    const mu_sign_t *matter,
    const mu_source_t *source)
  __attribute__((malloc, nonnull(1, 2, 3)));

#endif /* MU_SIGN_MEMBER_H */
