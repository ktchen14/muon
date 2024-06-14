#ifndef MU_STATOR_I
#define MU_STATOR_I

#include <muon/stator.h>  // IWYU pragma: export

#include <stddef.h>
#include <stdlib.h>

__attribute__((malloc, nonnull))
static inline void *stator_allocate(mu_engine_t *engine, size_t size) {
  return malloc(size);
}

#endif /* MU_STATOR_I */
