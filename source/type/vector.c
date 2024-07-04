#include "vector.h"

#include "../engine.h"
#include "../inductor.h"
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

  return assign_type(engine, result);
}

const mu_vector_type_t *vector_type_reduce(
    const mu_vector_type_t *type,
    mu_engine_t *engine,
    inductor_t *inductor) {
  const mu_type_t *matter = type->matter;
  inductor_member_t *matter_root = inductor_root(
    inductor, &(inductor_member_t) {
      .kind = INDUCTOR_TYPE,
      .id = matter->as_stator.id,
      .stator = &matter->as_stator,
    });
  assert(matter_root->kind == INDUCTOR_TYPE);

  const mu_type_t *matter_result = (const mu_type_t *) matter_root->stator;
  if (matter == matter_result)
    return type;

  return mu_vector_type(engine, matter_result);
}

void mu_vector_type_debug(const mu_vector_type_t *type) {
  putc('[', stderr);
  mu_type_debug(type->matter);
  putc(']', stderr);
}
