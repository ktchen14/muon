#ifndef MU_ENGINE_I
#define MU_ENGINE_I

#include <muon/engine.h>

#include "common.h"
#include "stator.h"

#include <stdlib.h>

typedef struct {
} engine_t;

static inline void *engine_allocate(mu_engine_t *engine, size_t size) {
  return malloc(size);
}

__attribute__((nonnull))
static inline mu_stator_t *engine_assign(
    mu_engine_t *engine, mu_stator_t *stator) {
  stator->engine = engine;
  return stator;
}

#define engine_assign_concrete(engine, stator) \
  ((typeof((stator))) engine_assign((engine), &(stator)->as_stator))

#endif /* MU_ENGINE_I */
