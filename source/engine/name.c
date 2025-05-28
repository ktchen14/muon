#include "name.h"

#include "common.h"

#include "../common.h"

#include <errno.h>
#include <stddef.h>
#include <string.h>

const mu_name_t *mu_name(
    mu_engine_t *engine,
    size_t length,
    const mu_char8_t text[restrict static length]) {
  for (size_t i = 0; i < engine->name_number; i++) {
    const mu_name_t *name = engine->name[i];

    if (length != name->length)
      continue;
    if (memcmp(text, name->text, length) != 0)
      continue;

    return name;
  }

  size_t size;
  if (rare((size = struct_size(mu_name_t, text, length + 1)) == 0))
    return errno = ENOMEM, NULL;

  mu_name_t *name;
  if ((name = engine_allocate(engine, size)) == NULL)
    return NULL;
  *name = (mu_name_t) { .engine = engine, .length = length };
  memcpy(name->text, text, length);
  name->text[length] = '\0';

  return engine->name[engine->name_number++] = name;
}

void mu_name_debug(const mu_name_t *name) {
  debug(PRIsNAME, DEBUG_NAME(name));
}
