#ifndef MU_STATOR_VECTOR_SIGN_H
#define MU_STATOR_VECTOR_SIGN_H

#include "abstract_node.h"

typedef struct {
  MU_SIGN_HEADER;

  const mu_sign_t *matter;
} mu_vector_sign_t;

const mu_vector_sign_t *mu_vector_sign(
    mu_engine_t *engine, const mu_sign_t *matter)
  __attribute__((malloc, nonnull));

void mu_vector_sign_debug(const mu_vector_sign_t *sign)
  __attribute__((nonnull));

#endif /* MU_STATOR_VECTOR_SIGN_H */
