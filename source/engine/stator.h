#ifndef MUON_ENGINE_STATOR_I
#define MUON_ENGINE_STATOR_I

#include <muon/engine/stator.h>

#include "common.h"

#include <stddef.h>

[[gnu::nonnull]] static inline const void *stator_next(
    const MuonEngine *opaque, MuonStatorTag tag, Hash hash, size_t *offset) {
  const Engine *engine = as_engine(opaque);
  return hash_search(engine->stator, hash << 8 | tag, offset);
}

/// Return the next stator assignable to @a stator in the iterator @a it
#define stator_next(engine, stator, hash, offset) ((typeof(stator)) { \
  stator_next((engine), MUON_STATOR_TAG(typeof(stator)), (hash), (offset)) \
})

/// Insert the @a stator into the @a engine at the @a offset
[[gnu::nonnull]] static inline const void *stator_insert(
    MuonEngine *opaque,
    const void *stator,
    MuonStatorTag tag,
    Hash hash,
    size_t offset) {
  Engine *engine = as_engine(opaque);

  HashArea *area = engine->stator;
  if ((area = hash_insert(area, hash << 8 | tag, stator, offset)) == NULL)
    return NULL;
  engine->stator = area;

  return stator;
}

#define stator_insert(engine, stator, hash, offset) __extension__ ({ \
  const typeof_unqual(*(stator)) *_stator = (stator); \
  MuonStatorTag _tag = MUON_STATOR_TAG(typeof(_stator)); \
  (typeof(_stator)) {stator_insert((engine), (stator), _tag, (hash), (offset))}; \
})

#endif /* MUON_ENGINE_STATOR_I */
