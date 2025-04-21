#include "area.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

void *area_allocate_internal(area_t **area, size_t area_size) {
  assert(area_size >= AREA_SIZE);

  area_t *next;
  if ((next = malloc(area_size)) == NULL)
    return NULL;
  (*area)->next = next;

  *next = (area_t) {
    .sentinel = &next->data[area_size - offsetof(area_t, data)],
    .volume = area_size - offsetof(area_t, data),
  };
  return next->data;
}
