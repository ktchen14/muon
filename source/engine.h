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

static inline mu_stator_t *engine_register(mu_engine_t *engine, mu_stator_t *stator) {
  stator->engine = engine;
  return stator;
}

#endif /* MU_ENGINE_I */
