#ifndef MU_ENGINE_I
#define MU_ENGINE_I

#include <muon/engine.h>

#include "common.h"
#include "stator.h"

typedef struct {
} engine_t;

__attribute__((nonnull))
static inline mu_stator_t *engine_assign(
    mu_engine_t *engine, mu_stator_t *stator) {
  stator->engine = engine;
  return stator;
}

#define engine_assign_concrete(engine, stator) \
  ((typeof((stator))) engine_assign((engine), &(stator)->as_stator))

#endif /* MU_ENGINE_I */
