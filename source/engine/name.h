#ifndef MUON_ENGINE_NAME_I
#define MUON_ENGINE_NAME_I

#include <muon/engine/name.h>

#include "common.h"

#include <stddef.h>
#include <string.h>

/// Literal printf specifier for a name
#define PRIsNAME "%s"

/// Used with PRIsNAME to emit the text of the @a name
#define DEBUG_NAME(name) ((name)->text)

/// Compare name @a a to name @a b
MUON_HINT(nonnull, pure)
static inline int name_cmp(MuonName *a, MuonName *b) {
  int result;
  if ((result = memcmp(a->text, b->text, minimum(a->length, b->length))) != 0)
    return result;
  return (a->length > b->length) - (b->length > a->length);
}

#endif /* MUON_ENGINE_NAME_I */
