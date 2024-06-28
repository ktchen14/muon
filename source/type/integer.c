#include "integer.h"

#include "../engine.h"

#include <stddef.h>
#include <stdio.h>

const mu_integer_type_t *mu_integer_type(mu_engine_t *engine) {
  size_t size = sizeof(mu_integer_type_t);

  mu_integer_type_t *result;
  if (rare((result = type_allocate(engine, size)) == NULL))
    return NULL;
  *result = (mu_integer_type_t) {
    .as_type.kind = MU_INTEGER_TYPE,
  };

  return engine_assign_concrete(engine, result);
}

void mu_integer_type_debug(const mu_integer_type_t *type) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Integer Type #%zu\n", type->as_stator.id);
}
