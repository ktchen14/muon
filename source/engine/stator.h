#ifndef MUON_ENGINE_STATOR_I
#define MUON_ENGINE_STATOR_I

#include <muon/engine/stator.h>

#include "common.h"

#include <stddef.h>
#include <stdint.h>

/// Return the hash code of the @a data with size @a size, when it follows some
/// data with hash code @a hash.
[[gnu::nonnull, gnu::pure]] static inline size_t hash_continue(
    size_t hash, const void *data, size_t size) {
  // TODO: make this work on 32-bit systems
  for (size_t i = 0; i < size; i++)
    hash = (hash ^ ((const char *) {data})[i]) * UINT64_C(1099511628211);
  return hash;
}

/// Return the hash code of the @a data with size @a size
[[gnu::nonnull, gnu::pure]] static inline size_t hash_string(
    const void *data, size_t size) {
  return hash_continue(UINT64_C(14695981039346656037), data, size);
}

/// Return the hash code of the @a object
#define hash_object(object) hash_string(&(object), sizeof(object))

/// Return hash code @a b joined to hash code @a a
[[gnu::const]] static inline size_t hash_join(size_t a, size_t b) {
  return b + UINT64_C(0x9e3779b97f4a7c15) + (a << 12) + (a >> 4);
}

[[gnu::nonnull, gnu::pure]] static inline size_t stator_slot(
    const Engine *engine, MuonStator *stator, size_t i) {
  size_t offset = i - stator->hash & engine->stator_volume - 1;
  for (MuonStator *next;; offset++) {
    i = stator->hash + offset & engine->stator_volume - 1;

    if ((next = engine->stator[i]) == NULL)
      return i;

    if ((i - next->hash & engine->stator_volume - 1) < offset)
      return i;
  }
}

[[gnu::nonnull]] static inline MuonStator *stator_next(
    const Engine *engine, MuonStatorTag tag, size_t hash, size_t *offset) {
  MuonStator stator = {.tag = tag, .hash = hash};
  for (MuonStator *next;; (*offset)++) {
    size_t i = stator.hash + *offset & engine->stator_volume - 1;

    if ((next = engine->stator[i]) == NULL)
      return NULL;

    if ((i - next->hash & engine->stator_volume - 1) < *offset)
      return NULL;

    if (next->tag == stator.tag && next->hash == stator.hash)
      return next;
  }
}

/// Return the next stator assignable to @a stator in the iterator @a it
#define stator_next(engine, stator, hash, offset) ((typeof(stator)) \
  stator_next((engine), MUON_STATOR_TAG(typeof(stator)), (hash), (offset)) \
)

/// Insert the @a stator into the @a engine at the @a offset
[[gnu::nonnull]] static inline MuonStator *stator_insert(
    Engine *engine, MuonStator *stator, size_t offset) {
  Engine *stator_rehash(Engine *engine, MuonStator *stator, size_t *i) //-
    MUON_HINT_SUFFIX(nonnull);

  size_t i;
  if (engine->stator_length < engine->stator_volume / 8 * 7)
    i = stator->hash + offset & engine->stator_volume - 1;
  else if (stator_rehash(engine, stator, &i) == NULL)
    return NULL;

  MuonStator *next = stator;
  while ((next = MOVE(engine->stator[i], next)) != NULL)
    i = stator_slot(engine, next, i + 1);
  return engine->stator_length++, stator;
}

#endif /* MUON_ENGINE_STATOR_I */
