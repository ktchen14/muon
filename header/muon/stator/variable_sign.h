#ifndef MU_STATOR_VARIABLE_SIGN_H
#define MU_STATOR_VARIABLE_SIGN_H

#include "abstract_node.h"

typedef struct {
  MU_SIGN_HEADER;
} mu_variable_sign_t;

const mu_variable_sign_t *mu_variable_sign(
    mu_engine_t *engine, const mu_node_source_t *source)
  __attribute__((malloc, nonnull(1)));

void mu_variable_sign_debug(const mu_variable_sign_t *sign)
  __attribute__((nonnull));

#endif /* MU_STATOR_VARIABLE_SIGN_H */
