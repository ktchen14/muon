#ifndef MU_STATOR_BOOLEAN_SIGN_H
#define MU_STATOR_BOOLEAN_SIGN_H

#include "abstract_node.h"

typedef struct {
  MU_SIGN_HEADER;
} mu_boolean_sign_t;

const mu_boolean_sign_t *mu_boolean_sign(mu_engine_t *engine)
  __attribute__((malloc, nonnull));

void mu_boolean_sign_debug(const mu_boolean_sign_t *sign)
  __attribute__((nonnull));

#endif /* MU_STATOR_BOOLEAN_SIGN_H */
