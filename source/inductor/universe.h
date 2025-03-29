#ifndef MU_INDUCTOR_UNIVERSE_I
#define MU_INDUCTOR_UNIVERSE_I

#include "coercion.h"
#include "type.h"

#include <assert.h>
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
  _Bool indirect : 1;
  _Bool transitive : 1;
} type_edge_t;

typedef struct {
  size_t length;
  size_t volume;
  type_edge_t *data;
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

type_edge_t *universe_search(
    const universe_t *universe, const mu_type_t *source, const mu_type_t *target)
  __attribute__((nonnull));

type_edge_t *append_edge(universe_t *universe, const mu_type_t *source, const mu_type_t *target);

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
    type_edge_t edge = universe->data[i];
    if (iterator->invert == 0 && edge.target == iterator->target)
      return edge.source;
    if (iterator->invert == 1 && edge.source == iterator->target)
      return edge.target;
  }

  return NULL;
}

__attribute__((nonnull))
static inline type_edge_t *universe_next(universe_iterator_t *iterator) {
  const universe_t *universe = iterator->universe;

  for (size_t i; (i = iterator->i++) < universe->length;) {
    type_edge_t *edge = &universe->data[i];
    if (iterator->invert == 0 && edge->target == iterator->target)
      return edge;
    if (iterator->invert == 1 && edge->source == iterator->target)
      return edge;
  }

  return NULL;
}

__attribute__((nonnull))
static inline const mu_coercion_t *coerce_with(const type_edge_t *edge) {
  // If the edge has a coercion, return it
  if (edge->coercion != NULL)
    return edge->coercion;

  // Otherwise, return an edge coercion for the edge
  const mu_edge_coercion_t *result;
  if ((result = mu_edge_coercion(edge->source, edge->target)) == NULL)
    return NULL;
  return ((type_edge_t *) edge)->coercion = &result->as_coercion;
}

__attribute__((nonnull, pure))
static inline const mu_coercion_t *course_coercion(const type_edge_t *edge) {
  const mu_coercion_t *result;
  if ((result = edge->coercion) == NULL || result->kind == MU_EDGE_COERCION)
    return NULL;
  return result;
}

__attribute__((nonnull))
static inline const mu_coercion_t *edge_assign(
    type_edge_t *edge, const mu_coercion_t *coercion) {
  assert(coercion->kind != MU_EDGE_COERCION);

  /* const mu_type_t *target = mu_coercion_target(coercion, edge->source); */
  /* const mu_variable_type_t *variable_target = mu_type_cast(target, variable_target); */
  /* assert(target == edge->target || */
  /*     variable_target != NULL && target == (const mu_type_t *) variable_target->solution); */

  if (coercion->kind == MU_INDIRECT_COERCION)
    edge->indirect = 1;

  return edge->coercion = coercion;
}

#endif /* MU_INDUCTOR_UNIVERSE_I */
