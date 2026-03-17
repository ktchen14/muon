#include "hash.h"
#include "common.h"

#include <errno.h>
#include <stdckdint.h>
#include <stddef.h>
#include <stdlib.h>

HashArea *rehash(HashArea *area, Hash hash, size_t *i) {
  size_t size;
  if (ckd_mul(&size, area->volume, sizeof(struct HashItem) * 2))
    return errno = ENOMEM, NULL;
  size_t volume = area->volume * 2;

  HashArea *allocation;
  if ((allocation = malloc(size)) == NULL)
    return NULL;
  for (size_t i = 0; i < volume; i++)
    allocation->item[i] = (struct HashItem) {};

  volume = MOVE(allocation->volume, volume);

  for (size_t j = 0, i; j < volume; j++) {
    struct HashItem next;
    if ((next = allocation->item[j]).data == NULL)
      continue;

    i = hash_slot(area, next.hash, next.hash);
    while ((next = MOVE(engine->stator[i], next)).stator != NULL)
      i = hash_slot(area, next.hash, i + 1);
  }

  *i = hash_slot(area, hash, hash);
  return area;
}
