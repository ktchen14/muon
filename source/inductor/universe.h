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
} universe_edge_t;

typedef struct {
  size_t length;
  size_t volume;
  universe_edge_t *data;
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

universe_edge_t *universe_search(
    const universe_t *universe, const mu_type_t *source, const mu_type_t *target)
  __attribute__((nonnull));

universe_edge_t *append_edge(universe_t *universe, const mu_type_t *source, const mu_type_t *target);

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
    universe_edge_t edge = universe->data[i];
    if (iterator->invert == 0 && edge.target == iterator->target)
      return edge.source;
    if (iterator->invert == 1 && edge.source == iterator->target)
      return edge.target;
  }

  return NULL;
}

__attribute__((nonnull))
static inline universe_edge_t *universe_next(universe_iterator_t *iterator) {
  const universe_t *universe = iterator->universe;

  for (size_t i; (i = iterator->i++) < universe->length;) {
    universe_edge_t *edge = &universe->data[i];
    if (iterator->invert == 0 && edge->target == iterator->target)
      return edge;
    if (iterator->invert == 1 && edge->source == iterator->target)
      return edge;
  }

  return NULL;
}

static inline const mu_coercion_t *edge_to_coercion(const universe_edge_t *edge) {
  // If the edge has a coercion, return it
  if (edge->coercion != NULL)
    return edge->coercion;

  // Otherwise, return an edge coercion for the edge
  const mu_edge_coercion_t *result;
  if ((result = mu_edge_coercion(edge->source, edge->target)) == NULL)
    return NULL;
  return ((universe_edge_t *) edge)->coercion = &result->as_coercion;
}

typedef universe_edge_t induce_edge_t;

#endif /* MU_INDUCTOR_UNIVERSE_I */
