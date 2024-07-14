#include "boolean_type.h"

#include "../engine.h"
#include "../stator.h"

#include <stddef.h>
#include <stdio.h>

const mu_boolean_type_t *mu_boolean_type(mu_engine_t *engine) {
  size_t size = sizeof(mu_boolean_type_t);

  mu_boolean_type_t *result;
  if ((result = type_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_boolean_type_t) {
    .as_type.kind = MU_BOOLEAN_TYPE,
  };

  return assign_type(engine, result);
}

void mu_boolean_type_debug(const mu_boolean_type_t *type) {
  fputs("Boolean", stderr);
}
