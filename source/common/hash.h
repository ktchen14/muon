#ifndef MUON_COMMON_HASH_I
#define MUON_COMMON_HASH_I

#include "common.h"

#include <stddef.h>
#include <stdint.h>

typedef size_t Hash;

typedef struct {
  size_t volume;
  size_t length;

  struct HashItem {
    Hash hash;
    const void *object;
  } item[];
} HashArea;

/// Extend the @a hash code with the @a data of size @a size
[[gnu::nonnull, gnu::pure]] static inline Hash hash_continue(
    Hash hash, const void *data, size_t size) {
  const char *string = data;
  // TODO: make this work on 32-bit systems
  for (size_t i = 0; i < size; i++)
    hash = (hash ^ string[i]) * UINT64_C(1099511628211);
  return hash;
}

/// Extend the @a hash code with the @a object
#define hash_extend(hash, object) \
  hash_continue((hash), &(object), sizeof(object))

#define HASH_ZERO UINT64_C(14695981039346656037)

/// Return the hash code of the @a data of size @a size
[[gnu::nonnull, gnu::pure]] static inline Hash hash_string(
    const void *data, size_t size) {
  return hash_continue(UINT64_C(14695981039346656037), data, size);
}

/// Return the hash code of the @a object
#define hash_object(object) hash_string(&(object), sizeof(object))

[[gnu::nonnull]] static inline const void *hash_search(
    const HashArea *area, Hash hash, size_t *offset) {
  for (struct HashItem next;; (*offset)++) {
    size_t i = hash + *offset & area->volume - 1;

    if ((next = area->item[i]).object == NULL)
      return NULL;

    if ((i - next.hash & area->volume - 1) < *offset)
      return NULL;

    if (next.hash == hash)
      return next.object;
  }
}

[[gnu::nonnull, gnu::pure]] static inline size_t hash_slot(
    const HashArea *area, Hash hash, size_t i) {
  size_t offset = i - hash & area->volume - 1;
  for (struct HashItem next;; offset++) {
    size_t i = hash + offset & area->volume - 1;

    if ((next = area->item[i]).object == NULL)
      return i;

    if ((i - next.hash & area->volume - 1) < offset)
      return i;
  }
}

/// Insert the @a stator into the @a engine at the @a offset
[[gnu::nonnull]] static inline HashArea *hash_insert(
    HashArea *area, Hash hash, const void *data, size_t offset) {
  HashArea *rehash(HashArea *area, Hash hash, size_t *i) //-
    MUON_HINT_SUFFIX(nonnull);

  size_t i;
  if (area->length < area->volume / 8 * 7)
    i = hash + offset & area->volume - 1;
  else if ((area = rehash(area, hash, &i)) == NULL)
    return NULL;

  struct HashItem next = {hash, data};
  while ((next = MOVE(area->item[i], next)).object != NULL)
    i = hash_slot(area, next.hash, i + 1);
  return area->length++, area;
}

#endif /* MUON_COMMON_HASH_I */
