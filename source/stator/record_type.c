#include "record_type.h"

#include "engine.h"
#include "name.h"
#include "type.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

const mu_record_type_t *mu_record_type(
    mu_engine_t *engine, size_t argc, const mu_type_member_t argv[argc]) {
  mu_record_type_t *result;
  if ((result = record_type_allocate(engine, argc)) == NULL)
    return NULL;
  memcpy(&result->argv, argv, sizeof(const mu_type_member_t[argc]));
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

  for (size_t i = 0; i < type->argc; i++) {
    const mu_name_t *member_name = type->argv[i].name;
    const mu_type_t *member_type = type->argv[i].type;
    assert(member_name == NULL || member_name->as_stator.engine == engine);
    assert(member_type->as_stator.engine == engine);
  }

  for (size_t i = 1; i < type->argc; i++) {
    const mu_name_t *a = type->argv[i - 1].name;
    const mu_name_t *b = type->argv[i - 0].name;
    assert(a == NULL || b != NULL && name_cmp(a, b) < 0);
  }

  mu_record_type_t source = {
    .as_type.kind = MU_RECORD_TYPE, .argc = type->argc
  };
  memcpy(type, &source, offsetof(mu_record_type_t, argv));

  return assign_type(engine, type);
}

const mu_record_type_t *record_type_import(
    const mu_record_type_t *type, const import_t *import) {
  mu_engine_t *engine = import->engine;

  mu_record_type_t *allocation;
  if ((allocation = record_type_allocate(engine, type->argc)) == NULL)
    return NULL;

  for (size_t i = 0; i < type->argc; i++) {
    // TODO
    const mu_name_t *member_name = type->argv[i].name;
    const mu_type_t *member_type = type->argv[i].type;
    member_type = import_retrieve(import->type, member_type);
    allocation->argv[i] = (mu_type_member_t) {
      .name = member_name, .type = member_type
    };
  }

  return record_type_activate(allocation);
}

static void type_member_debug(mu_type_member_t member) {
  if (member.name != NULL) {
    mu_name_debug(member.name);
    fputs(": ", stderr);
  }
  mu_type_debug(member.type);
}

void mu_record_type_debug(const mu_record_type_t *type) {
  putc('(', stderr);
  if (type->argc > 0) {
    type_member_debug(type->argv[0]);

    if (type->argc > 1) {
      for (size_t i = 1; i < type->argc; i++) {
        fputs(", ", stderr);
        type_member_debug(type->argv[i]);
      }
    } else
      putc(',', stderr);
  }
  putc(')', stderr);
}
