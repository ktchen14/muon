#ifndef MUON_ENGINE_STATOR_I
#define MUON_ENGINE_STATOR_I

#include <muon/engine/stator.h>

#include "common.h"
#include "../hash.h"

#include <stddef.h>
#include <stdint.h>

/// Return hash code @a b joined to hash code @a a
[[gnu::const]] static inline Hash hash_join(Hash a, Hash b) {
  return b + UINT64_C(0x9e3779b97f4a7c15) + (a << 12) + (a >> 4);
}

[[gnu::nonnull]] static inline const void *stator_next(
    const Engine *engine, MuonStatorTag tag, Hash hash, size_t *offset) {
  return hash_search(engine->stator, hash << 8 | tag, offset);
}

/// Return the next stator assignable to @a stator in the iterator @a it
#define stator_next(engine, stator, hash, offset) ((typeof(stator)) { \
  stator_next((engine), MUON_STATOR_TAG(typeof(stator)), (hash), (offset)) \
})

/// Insert the @a stator into the @a engine at the @a offset
[[gnu::nonnull]] static inline const void *stator_insert(
    Engine *engine, const void *stator, MuonStatorTag tag, Hash hash, size_t offset) {
  HashArea *area = engine->stator;
  if ((area = hash_insert(area, hash << 8 | tag, stator, offset)) == NULL)
    return NULL;
  engine->stator = area;

  return stator;
}

#endif /* MUON_ENGINE_STATOR_I */
