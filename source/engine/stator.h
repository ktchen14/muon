#ifndef MUON_ENGINE_STATOR_I
#define MUON_ENGINE_STATOR_I

#include "common.h"

#include <stddef.h>

/// Return the next stator in the @a engine with the @a hash code
[[gnu::nonnull]] static inline const void *stator_search(
    const MuonEngine *engine, Hash hash, size_t *offset) {
  return hash_search(as_engine(engine)->stator, hash, offset);
}

/// Insert the @a stator into the @a engine at the @a offset
[[gnu::nonnull]] static inline const void *stator_insert(
    MuonEngine *opaque, const void *stator, Hash hash, size_t offset) {
  Engine *engine = as_engine(opaque);
  HashVector *area = engine->stator;
  if ((area = hash_insert(area, hash, stator, offset)) == NULL)
    return NULL;
  return engine->stator = area, stator;
}

#endif /* MUON_ENGINE_STATOR_I */
