#ifndef MU_STATOR_BOOLEAN_TYPE_H
#define MU_STATOR_BOOLEAN_TYPE_H

#include "abstract_type.h"

typedef struct {
  MU_TYPE_HEADER;
} mu_boolean_type_t;

const mu_boolean_type_t *mu_boolean_type(mu_engine_t *engine)
  __attribute__((malloc, nonnull));

void mu_boolean_type_debug(const mu_boolean_type_t *type)
  __attribute__((nonnull));

#endif /* MU_STATOR_BOOLEAN_TYPE_H */
