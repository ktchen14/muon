#include "hash.h"
#include "common.h"

#include <errno.h>
#include <stdckdint.h>
#include <stddef.h>
#include <stdlib.h>

HashArea *rehash(HashArea *area, Hash hash, size_t *i) {
  size_t offset = offsetof(HashArea, item);
  size_t item = sizeof(struct HashItem[2]);
  size_t size = area->volume;
  if ((struct_size_overflow) (sizeof(HashArea), offset, item, &size))
    return errno = ENOMEM, NULL;

  HashArea *result;
  if ((result = malloc(size)) == NULL)
    return NULL;
  *result = (HashArea) {area->volume * 2, area->length};
  for (size_t i = 0; i < result->volume; i++)
    result->item[i] = (struct HashItem) {};

  for (size_t j = 0, i; j < area->volume; j++) {
    struct HashItem next;
    if ((next = area->item[j]).object == NULL)
      continue;

    i = hash_slot(result, next.hash, next.hash);
    while ((next = MOVE(result->item[i], next)).object != NULL)
      i = hash_slot(result, next.hash, i + 1);
  }
  free(area);

  return *i = hash_slot(result, hash, hash), result;
}
