#ifndef MU_ENGINE_COMMON_I
#define MU_ENGINE_COMMON_I

#include <muon/engine/common.h>  // IWYU pragma: export

#include <stddef.h>
#include <stdlib.h>

/// @internal Allocate an object of the @a size in the @a engine
__attribute__((malloc, nonnull))
static inline void *engine_allocate(mu_engine_t *engine, size_t size) {
  return malloc(size);
}

#endif /* MU_ENGINE_COMMON_I */
