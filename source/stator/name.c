#include "name.h"

#include "../common.h"
#include "engine.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* #include <uchar.h> */
#include <wchar.h>

size_t c8rtomb(char *restrict s, mu_char8_t c8, mbstate_t *restrict ps);

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

_Thread_local char conversion[MB_LEN_MAX];
_Thread_local mbstate_t cs = {0};

#include "debug.h"

void mu_name_debug(const mu_name_t *name) {
  if (debug_colorize)
    fputs("\x1b[0;32m", stderr);

  fprintf(stderr, "%s", name->text);

  /* for (size_t i = 0; i < name->length; i++) { */
  /*   mu_char8_t c = name->text[i]; */

  /*   size_t size; */
  /*   if ((size = c8rtomb(conversion, c, &cs)) == 0) */
  /*     continue; */
  /*   assert(size != (size_t) -1); */

  /*   fwrite(conversion, size, 1, stderr); */
  /* } */

  if (debug_colorize)
    fputs("\x1b[0m", stderr);

  assert(mbsinit(&cs));
}
