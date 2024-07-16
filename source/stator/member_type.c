#include "member_type.h"

#include "engine.h"
#include "name.h"
#include "type.h"
#include "../inductor.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

const mu_member_type_t *mu_member_type(
    mu_engine_t *engine, const mu_name_t *name, const mu_type_t *matter) {
  assert(name->as_stator.engine == engine);
  assert(matter->as_stator.engine == engine);

  size_t size = sizeof(mu_member_type_t);

  mu_member_type_t *result;
  if ((result = type_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_member_type_t) {
    .as_type.kind = MU_MEMBER_TYPE, .name = name, .matter = matter,
  };

  return assign_type(engine, result);
}

const mu_member_type_t *member_type_import(
    const mu_member_type_t *type, const import_t *import) {
  const mu_type_t *matter = type->matter;
  if ((matter = import_retrieve(import->type, matter)) == type->matter)
    return type;
  return mu_member_type(import->engine, type->name, matter);
}

const mu_member_type_t *member_type_reduce(
    const mu_member_type_t *type, inductor_t *inductor) {
  const mu_type_t *matter = type->matter;

  const mu_type_t *result;
  if ((result = reduce_type_result(inductor, type->matter)) == matter)
    return type;
  return mu_member_type(inductor->engine, type->name, result);
}

void mu_member_type_debug(const mu_member_type_t *type) {
  mu_name_debug(type->name);
  fputs(": ", stderr);
  mu_type_debug(type->matter);
}
