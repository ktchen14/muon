#ifndef MU_STATOR_I
#define MU_STATOR_I

#include <muon/stator.h>  // IWYU pragma: export

#include <stddef.h>
#include <stdlib.h>

__attribute__((malloc, nonnull))
static inline void *stator_allocate(mu_engine_t *engine, size_t size) {
  return malloc(size);
}

static inline _Bool stator_isnode(const mu_stator_t *stator) {
  switch (stator->kind) {
#define MU_EMIT(l, upper, t) case MU_##upper##_STATOR: return 1;
    MU_EACH_NODE_KIND(MU_EMIT)
#undef MU_EMIT

    default: return 0;
  }
}

static inline _Bool stator_istype(const mu_stator_t *stator) {
  switch (stator->kind) {
#define MU_EMIT(l, upper, t) case MU_##upper##_TYPE_STATOR: return 1;
    MU_EACH_TYPE_KIND(MU_EMIT)
#undef MU_EMIT

    default: return 0;
  }
}

#endif /* MU_STATOR_I */
