#ifndef MU_INDUCTOR_UNIVERSE_I
#define MU_INDUCTOR_UNIVERSE_I

#include "coercion.h"
#include "type.h"

#include <stddef.h>

typedef struct {
  const mu_type_t *source;
  const mu_type_t *target;

  const mu_coercion_t *coercion;

  _Bool direct;
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
static inline const mu_type_t *universe_next(universe_iterator_t *iterator) {
  const universe_t *universe = iterator->universe;
  for (size_t i = iterator->i++; i < universe->length; i++) {
    universe_edge_t edge = universe->data[i];
    if (!iterator->invert && edge.target == iterator->target)
      return edge.source;
    if (iterator->invert && edge.source == iterator->target)
      return edge.target;
  }

  return NULL;
}

typedef universe_edge_t induce_edge_t;

#endif /* MU_INDUCTOR_UNIVERSE_I */
