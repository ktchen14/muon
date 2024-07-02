#include "variable.h"

#include "../engine.h"
#include "../type.h"

#include <stddef.h>
#include <stdio.h>

const mu_variable_type_t *mu_variable_type(mu_engine_t *engine) {
  size_t size = sizeof(mu_variable_type_t);

  mu_variable_type_t *result;
  if ((result = type_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_variable_type_t) { .as_type.kind = MU_VARIABLE_TYPE };

  return engine_assign_concrete(engine, result);
}

void mu_variable_type_debug(const mu_variable_type_t *type) {
  fprintf(stderr, "Variable #%zu", type->as_stator.id);
}
