#ifndef MU_ENGINE_I
#define MU_ENGINE_I

#include <muon/engine.h>

#include "common.h"
#include "stator.h"

typedef struct {
} engine_t;

static inline mu_stator_t *engine_register(mu_engine_t *engine, mu_stator_t *stator) {
  stator->engine = engine;
  return stator;
}

#endif /* MU_ENGINE_I */
