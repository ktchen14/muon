#include "hash.h"
#include "common.h"

#include <assert.h>
#include <errno.h>
#include <stdckdint.h>
#include <stddef.h>
#include <stdlib.h>

HashVector *hash_vector(size_t volume) {
  assert((volume & volume - 1) == 0);

  size_t size = volume;
  if (struct_size_overflow(HashVector, item, &size))
    return errno = ENOMEM, NULL;

  HashVector *result;
  if ((result = malloc(size)) == NULL)
    return NULL;
  *result = (HashVector) {.volume = volume};
  for (size_t i = 0; i < volume; i++)
    result->item[i] = (struct HashItem) {};

  return result;
}

HashVector *hash_rehash(HashVector *vector, Hash hash, size_t *i) {
  size_t offset = offsetof(HashVector, item);
  size_t item = sizeof(struct HashItem[2]);

  size_t size = vector->volume;
  if ((struct_size_overflow) (sizeof(HashVector), offset, item, &size))
    return errno = ENOMEM, NULL;

  HashVector *result;
  if ((result = malloc(size)) == NULL)
    return NULL;
  *result = (HashVector) {vector->volume * 2, vector->length};
  for (size_t i = 0; i < result->volume; i++)
    result->item[i] = (struct HashItem) {};

  for (size_t j = 0, i; j < vector->volume; j++) {
    struct HashItem next;
    if ((next = vector->item[j]).object == NULL)
      continue;

    i = hash_slot(result, next.hash, next.hash);
    while ((next = MOVE(result->item[i], next)).object != NULL)
      i = hash_slot(result, next.hash, i + 1);
  }
  free(vector);

  return *i = hash_slot(result, hash, hash), result;
}
