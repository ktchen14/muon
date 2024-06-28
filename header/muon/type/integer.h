#ifndef MU_TYPE_INTEGER_H
#define MU_TYPE_INTEGER_H

#include "common.h"

typedef struct {
  MU_TYPE_HEADER;
} mu_integer_type_t;

const mu_integer_type_t *mu_integer_type(mu_engine_t *engine)
  __attribute__((malloc, nonnull));

void mu_integer_type_debug(const mu_integer_type_t *type)
  __attribute__((nonnull));

#endif /* MU_TYPE_INTEGER_H */
