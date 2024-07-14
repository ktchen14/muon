#ifndef MU_STATOR_VARIABLE_TYPE_H
#define MU_STATOR_VARIABLE_TYPE_H

#include "abstract_type.h"

typedef struct {
  MU_TYPE_HEADER;
} mu_variable_type_t;

const mu_variable_type_t *mu_variable_type(mu_engine_t *engine)
  __attribute__((malloc, nonnull));

void mu_variable_type_debug(const mu_variable_type_t *type)
  __attribute__((nonnull));

#endif /* MU_STATOR_VARIABLE_TYPE_H */
