#ifndef MU_INDUCTOR_UNIVERSE_I
#define MU_INDUCTOR_UNIVERSE_I

#include <muon/inductor/type.h>

#include "common.h"
#include "coercion.h"

#include <assert.h>
#include <stddef.h>

typedef struct {
  union {
    MUON_HINT(packed) struct {
      MuonType *source;
      MuonType *target;
    };

    MuonType *vertex[2];
  };

  MuonCoercion *coercion; // optional
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
  MuonType *target;
  _Bool invert;
  size_t i;
} universe_iterator_t;

/// Initialize the @a universe
universe_t *universe_initialize(universe_t *universe)
  MUON_HINT_SUFFIX(nonnull);

type_edge_t *universe_search(
    const universe_t *universe, MuonType *source, MuonType *target)
  MUON_HINT_SUFFIX(nonnull);

type_edge_t *append_edge(
    universe_t *universe, MuonType *source, MuonType *target);
type_edge_t *edge_define(
    universe_t *universe, MuonType *source, MuonType *target);

MUON_HINT(nonnull)
static inline universe_iterator_t universe_iterator(
    const universe_t *universe, MuonType *target, _Bool invert) {
  return (universe_iterator_t) {
    .universe = universe, .target = target, .invert = invert
  };
}

MUON_HINT(nonnull)
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

MUON_HINT(nonnull)
static inline MuonCoercion *coerce_with(
    mu_inductor_t *inductor, type_edge_t *edge) {
  // If the edge has a coercion, return it
  if (edge->coercion != NULL)
    return edge->coercion;

  // Otherwise, return an edge coercion for the edge
  MuonEdgeCoercion *result;
  if ((result = mu_edge_coercion(inductor, edge->target, edge->source)) == NULL)
    return NULL;
  return edge->coercion = &result->as_coercion;
}

MUON_HINT(nonnull, pure)
static inline MuonCoercion *course_coercion(const type_edge_t *edge) {
  MuonCoercion *result;
  if ((result = edge->coercion) == NULL || result->tag == MU_EDGE_COERCION)
    return NULL;
  return result;
}

MUON_HINT(nonnull)
static inline MuonCoercion *edge_assign(
    type_edge_t *edge, MuonCoercion *coercion) {
  assert(coercion->tag != MU_EDGE_COERCION);

  if (coercion->tag == MU_INDIRECT_COERCION)
    edge->indirect = 1;

  return edge->coercion = coercion;
}

#endif /* MU_INDUCTOR_UNIVERSE_I */
