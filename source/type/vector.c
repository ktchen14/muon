#include "vector.h"

#include "../engine.h"
#include "../type.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

const mu_vector_type_t *mu_vector_type(
    mu_engine_t *engine, const mu_type_t *matter) {
  assert(matter->as_stator.engine == engine);

  size_t size = sizeof(mu_vector_type_t);

  mu_vector_type_t *result;
  if ((result = type_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_vector_type_t) {
    .as_type.kind = MU_VECTOR_TYPE, .matter = matter,
  };

  return engine_assign_concrete(engine, result);
}

void mu_vector_type_debug(const mu_vector_type_t *type) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Vector Type #%zu:\n", type->as_stator.id);

  WITH_DEBUG_INDENT() { mu_type_debug(type->matter); }
}
