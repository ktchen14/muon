#include "name.h"

#include "common.h"
#include "stator.h"

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

MuonName *muon_name(
    MuonEngine *restrict opaque,
    size_t length,
    const char text[restrict static length]) {
  Engine *engine = as_engine(opaque);

  Hash hash = hash_join(
      hash_object((MuonStatorTag) {MUON_NAME_STATOR}),
      hash_string(text, length));

  MuonName *name;
  size_t i = 0;
  for (; (name = stator_next(engine, name, hash, &i)) != NULL; i++) {
    if (name->length == length && memcmp(name->text, text, length) == 0)
      return name;
  }

  size_t size = length + 1;
  if (struct_size_overflow(MuonName, text, &size))
    return errno = ENOMEM, NULL;

  struct MuonName *allocation;
  if ((allocation = engine_allocate2(engine, size)) == NULL)
    return NULL;
  *allocation = (MuonName) {.engine = opaque, .length = length};
  memcpy(allocation->text, text, length);
  allocation->text[length] = '\0';

  if (stator_insert(engine, allocation, MUON_NAME_STATOR, hash, i) == NULL)
    return NULL;
  return allocation;
}

void muon_name_debug(MuonName *name) {
  debug(PRIsNAME, DEBUG_NAME(name));
}
