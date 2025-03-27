#ifndef MU_INDUCTOR_UNIVERSE_I
#define MU_INDUCTOR_UNIVERSE_I

#include "coercion.h"
#include "type.h"

#include <stddef.h>

typedef struct {
  union {
    __attribute__((packed)) struct {
      const mu_type_t *source;
      const mu_type_t *target;
    };

    const mu_type_t *vertex[2];
  };

  const mu_coercion_t *coercion;  // optional
  int indirect;
} course_t;

typedef struct {
  size_t length;
  size_t volume;
  course_t *data;
} universe_t;

typedef struct {
  const universe_t *universe;
  const mu_type_t *target;
  _Bool invert;
  size_t i;
} universe_iterator_t;

/// Initialize the @a universe
universe_t *universe_initialize(universe_t *universe)
  __attribute__((nonnull));

course_t *course_search(
    const universe_t *universe, const mu_type_t *source, const mu_type_t *target)
  __attribute__((nonnull));

course_t *append_edge(universe_t *universe, const mu_type_t *source, const mu_type_t *target);

__attribute__((nonnull))
static inline universe_iterator_t universe_iterator(
    const universe_t *universe, const mu_type_t *target, _Bool invert) {
  return (universe_iterator_t) {
    .universe = universe, .target = target, .invert = invert,
  };
}

__attribute__((nonnull))
static inline const mu_type_t *universe_next_type(universe_iterator_t *iterator) {
  const universe_t *universe = iterator->universe;

  for (size_t i; (i = iterator->i++) < universe->length;) {
    course_t course = universe->data[i];
    if (iterator->invert == 0 && course.target == iterator->target)
      return course.source;
    if (iterator->invert == 1 && course.source == iterator->target)
      return course.target;
  }

  return NULL;
}

__attribute__((nonnull))
static inline course_t *universe_next(universe_iterator_t *iterator) {
  const universe_t *universe = iterator->universe;

  for (size_t i; (i = iterator->i++) < universe->length;) {
    course_t *course = &universe->data[i];
    if (iterator->invert == 0 && course->target == iterator->target)
      return course;
    if (iterator->invert == 1 && course->source == iterator->target)
      return course;
  }

  return NULL;
}

__attribute__((nonnull))
static inline const mu_coercion_t *coerce_as(const course_t *course) {
  // If the edge has a coercion, return it
  if (course->coercion != NULL)
    return course->coercion;

  // Otherwise, return an edge coercion for the course
  const mu_edge_coercion_t *result;
  if ((result = mu_edge_coercion(course->source, course->target)) == NULL)
    return NULL;
  return ((course_t *) course)->coercion = &result->as_coercion;
}

__attribute__((nonnull, pure))
const mu_coercion_t *course_coercion(const course_t *course) {
  const mu_coercion_t *result;
  if ((result = course->coercion) == NULL || result->kind == MU_EDGE_COERCION)
    return NULL;
  return result;
}

#endif /* MU_INDUCTOR_UNIVERSE_I */
