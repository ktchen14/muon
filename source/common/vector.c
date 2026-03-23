#include "vector.h"

#include "common.h"

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>

static inline _Bool vector_size_overflow(size_t member, size_t *size) {
  auto overflow = struct_size_overflow;
  size_t nought = sizeof(VectorHeader);
  size_t offset = offsetof(VectorHeader, data);
  return overflow(nought, offset, member, size);
}

void *(vector_allocate)(size_t member, size_t volume) { //-
  size_t size = volume;
  if (vector_size_overflow(member, &size))
    return errno = ENOMEM, NULL;

  VectorHeader *header;
  if ((header = malloc(size)) == NULL)
    return NULL;
  *header = (VectorHeader) {.volume = volume};

  return header->data;
}

void vector_delete(void *vector) {
  free(vector_header(vector));
}

void *(vector_resize)(void *vector, size_t member, size_t volume) { //-
  size_t size = volume;
  if (vector_size_overflow(member, &size))
    return errno = ENOMEM, NULL;

  VectorHeader *header = vector_header(vector);
  if ((header = realloc(header, size)) == NULL)
    return NULL;
  header->volume = volume;
  header->length = minimum(header->length, header->volume);
  return header->data;
}
