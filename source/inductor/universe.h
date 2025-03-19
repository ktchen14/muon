#ifndef MU_INDUCTOR_UNIVERSE_I
#define MU_INDUCTOR_UNIVERSE_I

#include "type.h"
#include "tactic.h"

#include <stddef.h>

typedef struct {
  union {
    const mu_type_t *source;
    const mu_type_t *lower;
  };

  union {
    const mu_type_t *target;
    const mu_type_t *upper;
  };

  const tactic_t *tactic;
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

/// Create an edge @a source → @a target in the @a universe
const universe_edge_t *universe_append(
    universe_t *universe,
    const mu_type_t *restrict source,
    const mu_type_t *restrict target,
    const tactic_t *tactic);

void mark_type_from_anywhere_first(
    const universe_t *universe, const mu_type_t *root, type_link_t *link);

typedef universe_edge_t induce_edge_t;

#endif /* MU_INDUCTOR_UNIVERSE_I */
