#include "record_type.h"

#include "engine.h"
#include "type.h"
#include "../inductor.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

const mu_record_type_t *mu_record_type(
    mu_engine_t *engine, size_t argc, const mu_type_t *const argv[argc]) {
  for (size_t i = 0; i < argc; i++)
    assert(argv[i]->as_stator.engine == engine);

  size_t size;
  if (rare((size = struct_size(mu_record_type_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_record_type_t *result;
  if ((result = type_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_record_type_t) {
    .as_type.kind = MU_RECORD_TYPE, .argc = argc,
  };
  memcpy(&result->argv, argv, sizeof(const mu_type_t *[argc]));

  return assign_type(engine, result);
}

const mu_record_type_t *record_type_reduce(
    const mu_record_type_t *type, inductor_t *inductor) {
  return NULL;
  /* const mu_type_t *matter = type->matter; */

  /* const mu_type_t *result; */
  /* if ((result = inductor_type_root(inductor, type->matter)) == matter) */
  /*   return type; */
  /* return mu_record_type(inductor->engine, result); */
}

void mu_record_type_debug(const mu_record_type_t *type) {
  putc('(', stderr);
  for (size_t i = 0; i < type->argc; i++)
    mu_type_debug(type->argv[i]);
  putc(')', stderr);
}
