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
    mu_engine_t *engine, size_t argc, const mu_test_t *argv[/* argc */]) {
  assert(argc == 0 && argv == NULL || argc != 0 && argv != NULL);
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

  if (argc != 0)
    memcpy(&result->argv, argv, sizeof(const mu_test_t *[argc]));

  return assign_type(engine, result);
}

const mu_variable_type_t *mu_open_type(mu_engine_t *engine) {
  return mu_variable_type(engine, 0, NULL);
}

mu_variable_type_t *variable_type_allocate(mu_engine_t *engine, size_t argc) {
  size_t size;
  if (rare((size = struct_size(mu_variable_type_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_variable_type_t *result;
  if ((result = type_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_variable_type_t) { .as_stator.engine = engine, .argc = argc };
  return result;
}

const mu_variable_type_t *variable_type_activate(mu_variable_type_t *type) {
  mu_engine_t *engine = (mu_engine_t *) type->as_stator.engine;

  for (size_t i = 0; i < type->argc; i++)
    assert(type->argv[i]->as_stator.engine == engine);

  mu_variable_type_t source = {
    .as_type.kind = MU_VARIABLE_TYPE, .argc = type->argc
  };
  memcpy(type, &source, offsetof(mu_variable_type_t, argv));
  return assign_type(engine, type);
}

static _Atomic size_t next_number = 0;
static const char *alphabet[] = {
  "α", "β", "γ", "δ", "ε", "ζ", "η", "θ", "ι", "κ", "μ", "ν", "ξ", "ο", "π",
  "ρ", "σ", "τ", "υ", "φ", "χ", "ψ", "ω" };
static size_t alphabet_length = sizeof(alphabet) / sizeof(alphabet[0]);

#include "../inductor/induce.h"

void mu_variable_type_debug(const mu_variable_type_t *type) {
  if (type->number == 0)
    ((mu_variable_type_t *) type)->number = ++next_number;

  static _Thread_local char buffer[256];

  char *name = buffer + sizeof(buffer);
  *--name = '\0';
  for (size_t n = type->number; n-- != 0; n /= alphabet_length) {
    const char *c = alphabet[n % alphabet_length];
    memcpy(name -= strlen(c), c, strlen(c));
  }
  fprintf(stderr, "%s", name);

  if (debug_induce == NULL)
    return;

  for (size_t i = 0; i < debug_induce->sub_length; i++) {
    induce_sub_t sub = debug_induce->sub_data[i];
    if (sub.upper != &type->as_type)
      continue;
    fprintf(stderr, " ⊓ ");
    mu_type_debug(sub.lower);
  }
}
