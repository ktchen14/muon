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
  mu_record_type_t *result;
  if ((result = record_type_allocate(engine, argc)) == NULL)
    return NULL;

  memcpy(&result->argv, argv, sizeof(const mu_type_t *[argc]));
  return record_type_activate(result);
}

mu_record_type_t *record_type_allocate(mu_engine_t *engine, size_t argc) {
  size_t size;
  if (rare((size = struct_size(mu_record_type_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_record_type_t *result;
  if ((result = type_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_record_type_t) { .as_stator.engine = engine, .argc = argc };
  return result;
}

const mu_record_type_t *record_type_activate(mu_record_type_t *type) {
  mu_engine_t *engine = (mu_engine_t *) type->as_stator.engine;

  for (size_t i = 0; i < type->argc; i++)
    assert(type->argv[i]->as_stator.engine == engine);

  mu_record_type_t source = {
    .as_type.kind = MU_RECORD_TYPE, .argc = type->argc
  };
  memcpy(type, &source, offsetof(mu_record_type_t, argv));

  return assign_type(engine, type);
}

const mu_record_type_t *record_type_reduce(
    const mu_record_type_t *type, inductor_t *inductor) {
  mu_engine_t *engine = inductor->engine;

  mu_record_type_t *allocation;
  if ((allocation = record_type_allocate(engine, type->argc)) == NULL)
    return NULL;

  for (size_t i = 0; i < type->argc; i++)
    allocation->argv[i] = inductor_root(inductor, type->argv[i]);

  return record_type_activate(allocation);
}

void mu_record_type_debug(const mu_record_type_t *type) {
  putc('(', stderr);
  if (type->argc > 0) {
    mu_type_debug(type->argv[0]);

    if (type->argc > 1) {
      for (size_t i = 1; i < type->argc; i++) {
        fputs(", ", stderr);
        mu_type_debug(type->argv[i]);
      }
    } else
      putc(',', stderr);
  }
  putc(')', stderr);
}
