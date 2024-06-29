#ifndef MU_TYPE_VARIABLE_H
#define MU_TYPE_VARIABLE_H

#include "common.h"

typedef struct {
  MU_TYPE_HEADER;
} mu_variable_type_t;

const mu_variable_type_t *mu_variable_type(mu_engine_t *engine)
  __attribute__((malloc, nonnull));

void mu_variable_type_debug(const mu_variable_type_t *type)
  __attribute__((nonnull));

#endif /* MU_TYPE_VARIABLE_H */
