#ifndef MUON_ENGINE_STATOR_I
#define MUON_ENGINE_STATOR_I

#include <muon/engine/stator.h>

#include "common.h"

#include <limits.h>
#include <stddef.h>

[[gnu::nonnull]] static inline const void *stator_search(
    const MuonEngine *engine, MuonStatorTag tag, Hash hash, size_t *offset) {
  hash = hash >> 8 | (Hash) tag << sizeof(Hash) * CHAR_BIT - 8;
  return hash_search(as_engine(engine)->stator, hash, offset);
}

/// Return the next stator assignable to @a stator in the iterator @a it
#define stator_search(engine, stator, hash, offset) ((typeof(stator)) { \
  stator_search((engine), MUON_STATOR_TAG(typeof(stator)), (hash), (offset)) \
})

/// Insert the @a stator into the @a engine at the @a offset
[[gnu::nonnull]] static inline const void *stator_insert(
    MuonEngine *opaque,
    const void *stator,
    MuonStatorTag tag,
    Hash hash,
    size_t offset) {
  hash = hash >> 8 | (Hash) tag << sizeof(Hash) * CHAR_BIT - 8;

  Engine *engine = as_engine(opaque);
  HashArea *area = engine->stator;
  if ((area = hash_insert(area, hash, stator, offset)) == NULL)
    return NULL;
  return engine->stator = area, stator;
}

#define stator_insert(engine, stator, hash, offset) __extension__ ({ \
  const typeof_unqual(*(stator)) *_stator = (stator); \
  MuonStatorTag _tag = MUON_STATOR_TAG(typeof(_stator)); \
  (typeof(_stator)) {stator_insert((engine), (stator), _tag, (hash), (offset))}; \
})

#endif /* MUON_ENGINE_STATOR_I */
