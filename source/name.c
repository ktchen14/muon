#include "name.h"

#include "common.h"
#include "engine.h"
#include "stator.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <uchar.h>
#include <wchar.h>

size_t c8rtomb(char *restrict s, char8_t c8, mbstate_t *restrict ps);

const mu_name_t *mu_name(
    mu_engine_t *engine,
    size_t length,
    const char8_t text[restrict static length]) {
  // Calculate the size of the name_header_t
  size_t size = offsetof(name_header_t, name.text) + 1;
  if (rare(__builtin_add_overflow(size, length, &size)))
    return errno = ENOMEM, NULL;
  size = maximum(size, sizeof(name_header_t));

  // Allocate and zero the header
  name_header_t *header;
  if ((header = stator_allocate(engine, size)) == NULL)
    return NULL;
  *header = (name_header_t) {0};

  mu_name_t *name = &header->name;
  *name = (mu_name_t) { .length = length };
  memcpy(name->text, text, length);
  name->text[length] = '\0';

  return engine_assign_concrete(engine, name);
}

_Thread_local char conversion[MB_LEN_MAX];
_Thread_local mbstate_t cs = {0};

void mu_name_debug(const mu_name_t *name) {
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
