#ifndef MU_STATOR_ENGINE_I
#define MU_STATOR_ENGINE_I

#include <muon/stator/engine.h>  // IWYU pragma: export

#include <stddef.h>
#include <stdlib.h>

/// @internal Allocate a stator of the @a size in the @a engine
__attribute__((malloc, nonnull))
static inline void *engine_allocate(mu_engine_t *engine, size_t size) {
  return malloc(size);
}

#endif /* MU_STATOR_ENGINE_I */
