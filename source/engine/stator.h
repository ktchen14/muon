#ifndef MUON_ENGINE_STATOR_I
#define MUON_ENGINE_STATOR_I

#include <muon/engine/stator.h>

#include "common.h"

#include <stddef.h>
#include <stdint.h>

/// Extend @a hash with the hash code of the @a data with size @a size
[[gnu::nonnull, gnu::pure]] static inline Hash hash_continue(
    Hash hash, const void *data, size_t size) {
  // TODO: make this work on 32-bit systems
  for (size_t i = 0; i < size; i++)
    hash = (hash ^ ((const char *) {data})[i]) * UINT64_C(1099511628211);
  return hash;
}

/// Return the hash code of the @a data with size @a size
[[gnu::nonnull, gnu::pure]] static inline Hash hash_string(
    const void *data, size_t size) {
  return hash_continue(UINT64_C(14695981039346656037), data, size);
}

/// Return the hash code of the @a object
#define hash_object(object) hash_string(&(object), sizeof(object))

/// Return hash code @a b joined to hash code @a a
[[gnu::const]] static inline Hash hash_join(Hash a, Hash b) {
  return b + UINT64_C(0x9e3779b97f4a7c15) + (a << 12) + (a >> 4);
}

[[gnu::nonnull, gnu::pure]] static inline size_t stator_slot(
    const Engine *engine, Hash hash, size_t i) {
  size_t offset = i - hash & engine->stator_volume - 1;
  for (struct HashStator next;; offset++) {
    i = hash + offset & engine->stator_volume - 1;

    if ((next = engine->stator[i]).stator == NULL)
      return i;

    if ((i - next.hash & engine->stator_volume - 1) < offset)
      return i;
  }
}

[[gnu::nonnull]] static inline MuonStator *stator_next(
    const Engine *engine, MuonStatorTag tag, Hash hash, size_t *offset) {
  struct HashStator stator = {.tag = tag, .hash = hash};
  for (struct HashStator next;; (*offset)++) {
    size_t i = stator.hash + *offset & engine->stator_volume - 1;

    if ((next = engine->stator[i]).stator == NULL)
      return NULL;

    if ((i - next.hash & engine->stator_volume - 1) < *offset)
      return NULL;

    if (next.tag == stator.tag && next.hash == stator.hash)
      return next.stator;
  }
}

/// Return the next stator assignable to @a stator in the iterator @a it
#define stator_next(engine, stator, hash, offset) ( \
  _Pragma("GCC diagnostic push") \
  _Pragma("GCC diagnostic ignored \"-Wcast-align\"") \
  (typeof(stator)) \
    stator_next((engine), MUON_STATOR_TAG(typeof(stator)), (hash), (offset)) \
  _Pragma("GCC diagnostic pop") \
)

/// Insert the @a stator into the @a engine at the @a offset
[[gnu::nonnull]] static inline MuonStator *stator_insert(
    Engine *engine, MuonStator *stator, Hash hash, size_t offset) {

  Engine *stator_rehash(
      Engine *engine, MuonStator *stator, Hash hash, size_t *i)
    MUON_HINT_SUFFIX(nonnull);

  size_t i;
  if (engine->stator_length < engine->stator_volume / 8 * 7)
    i = hash + offset & engine->stator_volume - 1;
  else if (stator_rehash(engine, stator, hash, &i) == NULL)
    return NULL;

  struct HashStator next = {.tag = stator->tag, .hash = hash, .stator = stator};
  while ((next = MOVE(engine->stator[i], next)).stator != NULL)
    i = stator_slot(engine, next.hash, i + 1);
  return engine->stator_length++, stator;
}

#endif /* MUON_ENGINE_STATOR_I */
