#include "area.h"

#include "common.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>

__attribute__((noinline))
area_t *area_create(area_t *area) {
  area_t *next;
  if ((next = malloc(AREA_SIZE)) == NULL)
    return NULL;
  *next = (area_t) {
    .volume = AREA_VOLUME, .sentinel = &next->data[AREA_VOLUME],
  };

  if (area != NULL)
    area->next = next;
  return next;
}

__attribute__((noinline))
void *area_create_allocate(area_t **area, size_t size) {
  area_t *next;
  if ((next = area_create(*area)) == NULL)
    return NULL;
  *area = next;
  next->volume -= size;
  return next->data;
}

__attribute__((noinline))
void *area_create_single(area_t **area, size_t size) {
  size_t area_size;
  if (rare((area_size = struct_size(area_t, data, size)) == 0))
    return errno = ENOMEM, NULL;

  area_t *next;
  if ((next = malloc(area_size)) == NULL)
    return NULL;


  size_t volume = area_size - offsetof(area_t, data);
  *next = (area_t) {
    .volume = volume - size, .sentinel = &next->data[volume],
  };

  (*area)->next = next;
  *area = next;
  return next->data;
}
