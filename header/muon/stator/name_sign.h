#ifndef MU_STATOR_NAME_SIGN_H
#define MU_STATOR_NAME_SIGN_H

#include "abstract_node.h"

#include "name.h"

typedef struct {
  MU_SIGN_HEADER;

  const mu_name_t *name;
} mu_name_sign_t;

const mu_name_sign_t *mu_name_sign(
    mu_engine_t *engine, const mu_name_t *name)
  __attribute__((malloc, nonnull));

void mu_name_sign_debug(const mu_name_sign_t *sign)
  __attribute__((nonnull));

#endif /* MU_STATOR_NAME_SIGN_H */
