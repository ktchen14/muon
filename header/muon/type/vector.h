#ifndef MU_TYPE_VECTOR_H
#define MU_TYPE_VECTOR_H

#include "common.h"

typedef struct {
  MU_TYPE_HEADER;

  const mu_type_t *matter;
} mu_vector_type_t;

const mu_vector_type_t *mu_vector_type(
    mu_engine_t *engine, const mu_type_t *matter)
  __attribute__((malloc, nonnull));

void mu_vector_type_debug(const mu_vector_type_t *type)
  __attribute__((nonnull));

#endif /* MU_TYPE_VECTOR_H */
