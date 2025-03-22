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

/// Initialize the @a universe
universe_t *universe_initialize(universe_t *universe)
  __attribute__((nonnull));

const universe_edge_t *universe_search(
    const universe_t *universe, const mu_type_t *source, const mu_type_t *target)
  __attribute__((nonnull));

universe_edge_t *append_edge(universe_t *universe, const mu_type_t *source, const mu_type_t *target);

typedef universe_edge_t induce_edge_t;

#endif /* MU_INDUCTOR_UNIVERSE_I */
