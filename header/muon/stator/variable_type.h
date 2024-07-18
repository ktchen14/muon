#ifndef MU_STATOR_VARIABLE_TYPE_H
#define MU_STATOR_VARIABLE_TYPE_H

#include "abstract_type.h"
#include "abstract_test.h"

typedef struct {
  MU_TYPE_HEADER;

  size_t number;
  size_t argc;
  const mu_test_t *argv[/* argc */];
} mu_variable_type_t;

const mu_variable_type_t *mu_variable_type(
    mu_engine_t *engine, size_t argc, const mu_test_t *argv[argc])
  __attribute__((malloc, nonnull(1)));

const mu_variable_type_t *mu_open_type(mu_engine_t *engine)
  __attribute__((malloc, nonnull));

void mu_variable_type_debug(const mu_variable_type_t *type)
  __attribute__((nonnull));

#endif /* MU_STATOR_VARIABLE_TYPE_H */
