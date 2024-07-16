#include "vector_type.h"

#include "engine.h"
#include "type.h"

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

  return assign_type(engine, result);
}

const mu_vector_type_t *vector_type_import(
    const mu_vector_type_t *type, const import_t *import) {
  const mu_type_t *matter = type->matter;
  if ((matter = import_retrieve(import->type, matter)) == type->matter)
    return type;
  return mu_vector_type(import->engine, matter);
}

void mu_vector_type_debug(const mu_vector_type_t *type) {
  putc('[', stderr);
  mu_type_debug(type->matter);
  putc(']', stderr);
}
