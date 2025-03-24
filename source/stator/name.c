#include "name.h"

#include "../common.h"
#include "engine.h"

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

const mu_name_t *mu_name(
    mu_engine_t *engine,
    size_t length,
    const mu_char8_t text[restrict static length]) {
  size_t size;
  if (rare((size = struct_size(mu_name_t, text, length + 1)) == 0))
    return errno = ENOMEM, NULL;

  mu_name_t *name;
  if ((name = engine_allocate(engine, size)) == NULL)
    return NULL;
  *name = (mu_name_t) { .length = length };
  memcpy(name->text, text, length);
  name->text[length] = '\0';

  for (size_t i = 0; i < engine->name_number; i++) {
    const mu_name_t *already = engine->name[i];
    if (name->length != already->length)
      continue;
    if (memcmp(name->text, already->text, name->length))
      continue;

    free(name);
    return already;
  }

  name->engine = engine;
  engine->name[engine->name_number++] = name;
  return name;
}

void mu_name_debug(const mu_name_t *name) {
  debug(PRIsNAME, DEBUG_NAME(name));
}
