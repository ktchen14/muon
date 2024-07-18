#ifndef MU_STATOR_LAMBDA_TYPE_H
#define MU_STATOR_LAMBDA_TYPE_H

#include "abstract_type.h"

typedef struct {
  MU_TYPE_HEADER;

  const mu_type_t *argument;
  const mu_type_t *output;
} mu_lambda_type_t;

const mu_lambda_type_t *mu_lambda_type(
    mu_engine_t *engine, const mu_type_t *argument, const mu_type_t *output)
  __attribute__((malloc, nonnull));

void mu_lambda_type_debug(const mu_lambda_type_t *type)
  __attribute__((nonnull));

#endif /* MU_STATOR_LAMBDA_TYPE_H */
