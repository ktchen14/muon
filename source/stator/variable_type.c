#include "variable_type.h"

#include "engine.h"
#include "type.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

const mu_variable_type_t *mu_variable_type(mu_engine_t *engine) {
  size_t size = sizeof(mu_variable_type_t);

  mu_variable_type_t *result;
  if ((result = type_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_variable_type_t) {
    .as_type.kind = MU_VARIABLE_TYPE,
  };

  return assign_type(engine, result);
}

static _Atomic size_t next_number = 0;
static const char *alphabet[] = {
  "α", "β", "γ", "δ", "ε", "ζ", "η", "θ", "ι", "κ", "μ", "ν", "ξ", "ο", "π",
  "ρ", "σ", "τ", "υ", "φ", "χ", "ψ", "ω" };
static size_t alphabet_length = sizeof(alphabet) / sizeof(alphabet[0]);

#include "debug.h"
#include "../inductor/induce.h"

void mu_variable_type_debug(const mu_variable_type_t *type) {
  // Assign the variable type a number
  if (type->number == 0)
    ((mu_variable_type_t *) type)->number = ++next_number;

  static _Thread_local char buffer[256];

  // Generate a name
  char *name = buffer + sizeof(buffer);
  *--name = '\0';
  for (size_t n = type->number; n-- != 0; n /= alphabet_length) {
    const char *c = alphabet[n % alphabet_length];
    memcpy(name -= strlen(c), c, strlen(c));
  }

  if (debug_induce == NULL)
    return;

  _Bool already_printed = 0;
  if (debug_negate) {
    for (size_t i = 0; i < debug_induce->sub_length; i++) {
      induce_sub_t sub = debug_induce->sub_data[i];
      if (sub.lower != &type->as_type)
        continue;
      if (already_printed)
        fprintf(stderr, " ⊓ ");
      already_printed = 1;
      mu_type_debug(sub.upper);
    }

    if (!already_printed)
      fprintf(stderr, "%s", name);
  } else {
    for (size_t i = 0; i < debug_induce->sub_length; i++) {
      induce_sub_t sub = debug_induce->sub_data[i];
      if (sub.upper != &type->as_type)
        continue;
      if (already_printed > 0)
        fprintf(stderr, " ⊔ ");
      already_printed = 1;
      mu_type_debug(sub.lower);
    }

    if (!already_printed)
      fprintf(stderr, "%s", name);
  }
}
