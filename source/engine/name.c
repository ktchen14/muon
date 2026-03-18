#include "name.h"

#include "common.h"
#include "stator.h"

#include <errno.h>
#include <stddef.h>
#include <string.h>

MuonName *muon_name(
    MuonEngine *restrict engine,
    size_t length,
    const char text[restrict static length]) {
  Hash hash = hash_string(text, length);

  MuonName *next;
  size_t i = 0;
  for (; (next = stator_next(engine, next, hash, &i)) != NULL; i++) {
    if (next->length == length && memcmp(next->text, text, length) == 0)
      return next;
  }

  size_t size = length + 1;
  if (struct_size_overflow(MuonName, text, &size))
    return errno = ENOMEM, NULL;

  struct MuonName *name;
  if ((name = engine_allocate(engine, size)) == NULL)
    return NULL;
  *name = (MuonName) {.engine = engine, .length = length};
  memcpy(name->text, text, length);
  name->text[length] = '\0';

  return stator_insert(engine, name, hash, i);
}

void muon_name_debug(MuonName *name) {
  debug(PRIsNAME, DEBUG_NAME(name));
}
