#include "name.h"

#include "common.h"

#include "../common.h"

#include <errno.h>
#include <stddef.h>
#include <string.h>

MuonName *muon_name(
    MuonEngine *engine,
    size_t length,
    const char text[restrict static length]) {
  for (size_t i = 0; i < engine->name_number; i++) {
    MuonName *name = engine->name[i];

    if (length != name->length)
      continue;
    if (memcmp(text, name->text, length) != 0)
      continue;

    return name;
  }

  size_t size;
  if (rare((size = struct_size(MuonName, text, length + 1)) == 0))
    return errno = ENOMEM, NULL;

  struct MuonName *name;
  if ((name = engine_allocate(engine, size)) == NULL)
    return NULL;
  *name = (MuonName) { .engine = engine, .length = length };
  memcpy(name->text, text, length);
  name->text[length] = '\0';

  return engine->name[engine->name_number++] = name;
}

void muon_name_debug(MuonName *name) {
  debug(PRIsNAME, DEBUG_NAME(name));
}
