#include "universe.h"

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>

universe_t *universe_initialize(universe_t *universe) {
  universe_edge_t *data;
  size_t volume = 1;
  if ((data = malloc(sizeof(universe_edge_t[volume]))) == NULL)
    return NULL;
  *universe = (universe_t) { .volume = 1, .data = data };
  return universe;
}

const universe_edge_t *universe_search(
    const universe_t *universe, const mu_type_t *source, const mu_type_t *target) {
  for (size_t i = 0; i < universe->length; i++) {
    const universe_edge_t *edge = &universe->data[i];
    if (edge->source == source && edge->target == target)
      return edge;
  }
  return NULL;
}

const universe_edge_t *universe_append(
    universe_t *universe,
    const mu_type_t *restrict source,
    const mu_type_t *restrict target,
    _Bool direct,
    const tactic_t *tactic) {
  if (universe->length >= universe->volume) {
    size_t volume = universe->volume;
    if (rare(__builtin_mul_overflow(volume, 2, &volume)))
      return errno = ENOMEM, NULL;

    size_t size;
    if (rare(__builtin_mul_overflow(volume, sizeof(universe_edge_t), &size)))
      return errno = ENOMEM, NULL;

    universe_edge_t *data;
    if ((data = realloc(universe->data, size)) == NULL)
      return NULL;
    universe->data = data;

    universe->volume = volume;
  }

  universe_edge_t *result = &universe->data[universe->length++];
  *result = (universe_edge_t) {
    .source = source, .target = target, .direct = direct, .tactic = tactic,
  };
  return result;
}
