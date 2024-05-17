#include "name.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const mu_name_t *mu_name(
    mu_engine_t *engine, const char8_t *restrict text, size_t length) {
  size_t text_size;
  if (rare(__builtin_add_overflow(length, 1, &text_size)))
    return errno = ENOMEM, NULL;

  size_t size;
  if (rare((size = struct_size(mu_name_t, text, text_size)) == 0))
    return errno = ENOMEM, NULL;

  mu_name_t *name;
  if ((name = malloc(size)) == NULL)
    return NULL;

  *name = (mu_name_t) { .engine = engine, .length = length };
  memcpy(name->text, text, length);
  name->text[length] = '\0';
  return name;
}

void mu_name_debug(const mu_name_t *name) {
  fprintf(stderr, "%s", name->text);
}
