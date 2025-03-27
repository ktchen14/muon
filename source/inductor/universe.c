#include "universe.h"

#include "../common.h"
#include "type.h"

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>

universe_t *universe_initialize(universe_t *universe) {
  course_t *data;
  size_t volume = 1000;
  if ((data = malloc(sizeof(course_t[volume]))) == NULL)
    return NULL;
  *universe = (universe_t) { .volume = volume, .data = data };
  return universe;
}

course_t *universe_search(
    const universe_t *universe, const mu_type_t *source, const mu_type_t *target) {
  for (size_t i = 0; i < universe->length; i++) {
    course_t *course = &universe->data[i];
    if (course->source == source && course->target == target)
      return course;
  }
  return NULL;
}

course_t *append_edge(universe_t *universe, const mu_type_t *source, const mu_type_t *target) {
  if (universe->length >= universe->volume) {
    size_t volume = universe->volume;
    if (rare(__builtin_mul_overflow(volume, 2, &volume)))
      return errno = ENOMEM, NULL;

    size_t size;
    if (rare(__builtin_mul_overflow(volume, sizeof(course_t), &size)))
      return errno = ENOMEM, NULL;

    course_t *data;
    if ((data = realloc(universe->data, size)) == NULL)
      return NULL;
    universe->data = data;

    universe->volume = volume;
  }

  course_t *result = &universe->data[universe->length++];
  *result = (course_t) { .source = source, .target = target };
  return result;
}
