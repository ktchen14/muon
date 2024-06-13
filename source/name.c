#include "name.h"

#include "engine.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uchar.h>
#include <wchar.h>

size_t c8rtomb(char *restrict s, char8_t c8, mbstate_t *restrict ps);

const mu_name_t *mu_name(
    mu_engine_t *engine, const char8_t *restrict text, size_t length) {
  size_t text_size;
  if (rare(__builtin_add_overflow(length, 1, &text_size)))
    return errno = ENOMEM, NULL;

  /* size_t size; */
  /* if (rare((size = struct_size(mu_name_t, text, text_size)) == 0)) */
  /*   return errno = ENOMEM, NULL; */

  size_t size;
  if (rare((size = struct_size(name_header_t, name.text, text_size)) == 0))
    return errno = ENOMEM, NULL;

  name_header_t *header;
  if ((header = malloc(size)) == NULL)
    return NULL;
  header->cursor = (name_cursor_t) {0};

  mu_name_t *name = &header->name;
  *name = (mu_name_t) { .length = length };
  memcpy(name->text, text, length);
  name->text[length] = '\0';

  return (mu_name_t *) engine_register(engine, &name->as_stator);
}

_Thread_local char conversion[MB_LEN_MAX];
_Thread_local mbstate_t cs = {0};

void mu_name_debug(const mu_name_t *name) {
  /* fprintf(stderr, "%s", name->text); */

  for (size_t i = 0; i < name->length; i++) {
    char8_t c = name->text[i];

    size_t size;
    if ((size = c8rtomb(conversion, c, &cs)) == 0)
      continue;
    assert(size != (size_t) -1);

    fwrite(conversion, size, 1, stderr);
  }

  assert(mbsinit(&cs));
}
