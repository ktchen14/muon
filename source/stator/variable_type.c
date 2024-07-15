#include "variable_type.h"

#include "engine.h"
#include "test.h"
#include "type.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

const mu_variable_type_t *mu_variable_type(
    mu_engine_t *engine, size_t argc, const mu_test_t *argv[argc]) {
  for (size_t i = 0; i < argc; i++)
    assert(argv[i]->as_stator.engine == engine);

  size_t size;
  if (rare((size = struct_size(mu_variable_type_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_variable_type_t *result;
  if ((result = type_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_variable_type_t) {
    .as_type.kind = MU_VARIABLE_TYPE, .argc = argc,
  };
  memcpy(&result->argv, argv, sizeof(const mu_test_t *[argc]));

  return assign_type(engine, result);
}

const mu_variable_type_t *mu_open_type(mu_engine_t *engine) {
  return mu_variable_type(engine, 0, NULL);
}

void mu_variable_type_debug(const mu_variable_type_t *type) {
  fprintf(stderr, "Variable #%zu", type->as_stator.id);
}
