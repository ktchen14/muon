/// @file source/common/vector.h

#ifndef MUON_COMMON_VECTOR_I
#define MUON_COMMON_VECTOR_I

#include "common.h"

#include <errno.h>
#include <stdckdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/// Type of a vector with element type @a T
#define Vector(T) typeof(&(T) {})

typedef struct {
  size_t volume;
  size_t length;
  _Alignas(max_align_t) char data[];
} VectorHeader;

/// Return the header of the @a vector
static inline VectorHeader *vector_header(const void *vector) {
  const size_t offset = offsetof(VectorHeader, data);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wcast-qual"
  return (VectorHeader *) ((char *) vector - offset);
#pragma GCC diagnostic pop
}

/// Return the volume of the @a vector
[[gnu::nonnull, gnu::pure]] static inline size_t vector_volume(
    const void *vector) {
  return vector_header(vector)->volume;
}

/// Return the length of the @a vector
[[gnu::nonnull, gnu::pure]] static inline size_t vector_length(
    const void *vector) {
  return vector_header(vector)->length;
}

void *vector_allocate(size_t member, size_t volume);

#define vector_allocate(T, volume) ( \
  (Vector(T)) {vector_allocate(sizeof((T) {}), (volume))} \
)

void vector_delete(void *vector);

void *vector_resize(void *vector, size_t member, size_t volume);

#define vector_resize(vector, volume) ( \
  (typeof(vector)) {vector_resize((vector), sizeof(*(vector)), (volume))} \
)

MUON_HINT(nonnull) static inline void *vector_ensure(
    void *vector, size_t member, size_t length) {
  auto resize = vector_resize;

  if (length <= vector_volume(vector))
    return vector;

  // just volume = (length * 8 + 3) / 5 avoiding intermediate overflow
  size_t volume = length / 5 * 8 + ((length % 5) * 8 + 3) / 5;

  // if the volume doesn't overflow then attempt to allocate it
  if (volume > length) {
    void *result;
    if ((result = resize(vector, member, volume)) != NULL)
      return result;
  }

  // if either the volume overflows or the allocation failed then attempt to
  // resize to just the length
  return resize(vector, member, length);
}

#define vector_ensure(vector, length) ( \
  (typeof(vector)) {vector_ensure((vector), sizeof(*(vector)), (length))} \
)

static inline void *vector_insert(
    void *vector, size_t member, const void *data, size_t i) {
  auto ensure = vector_ensure;

  size_t length;
  if (rare(ckd_add(&length, vector_length(vector), 1)))
    return errno = ENOMEM, NULL;

  if ((vector = ensure(vector, member, length)) == NULL)
    return NULL;

  // move the existing elements n elements toward the tail
  void *target = &(char *) {vector}[member * (i + 1)];
  void *source = &(char *) {vector}[member * (i + 0)];
  memmove(target, source, member * (vector_length(vector) - i));

  if (data != NULL)
    memcpy(&(char *) {vector}[member * i], data, member);

  return vector_header(vector)->length = length, vector;
}

#define vector_insert(vector, data, i) ((typeof(vector)) {vector_insert( \
  (vector), sizeof(*(vector)), (const typeof(*(vector)) *) {(data)}, (i) \
)})

static inline void *vector_append(
    void *vector, size_t member, const void *data) {
  auto insert = vector_insert;
  return insert(vector, member, data, vector_length(vector));
}

#define vector_append(vector, data) ((typeof(vector)) {vector_append( \
  (vector), sizeof(*(vector)), (const typeof(*(vector)) *) {(data)} \
)})

#endif /* MUON_COMMON_VECTOR_I */
