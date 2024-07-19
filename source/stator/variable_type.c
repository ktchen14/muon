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

static _Atomic size_t next_number = 0;
static const char *alphabet[] = {
  "α", "β", "γ", "δ", "ε", "ζ", "η", "θ", "ι", "κ", "μ", "ν", "ξ", "ο", "π",
  "ρ", "σ", "τ", "υ", "φ", "χ", "ψ", "ω" };
static size_t alphabet_length = sizeof(alphabet) / sizeof(alphabet[0]);

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

  if (type->argc > 0) {
    fputs(" with (", stderr);
    for (size_t i = 0; i < type->argc; i++) {
      if (i > 1)
        fputs(", ", stderr);
      mu_test_debug(type->argv[i]);
    }
    putc(')', stderr);
  }
}
