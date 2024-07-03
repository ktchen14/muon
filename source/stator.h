#ifndef MU_STATOR_I
#define MU_STATOR_I

#include <muon/stator.h>  // IWYU pragma: export

#include <stddef.h>

static inline _Bool stator_isnode(const mu_stator_t *stator) {
  switch (stator->kind) {
#define MU_EMIT(lower, upper, t) case MU_##upper##_STATOR: return 1;
    MU_EACH_NODE_KIND(MU_EMIT)
#undef MU_EMIT

    default: return 0;
  }
}

static inline _Bool stator_istype(const mu_stator_t *stator) {
  switch (stator->kind) {
#define MU_EMIT(lower, upper, t) case MU_##upper##_TYPE_STATOR: return 1;
    MU_EACH_TYPE_KIND(MU_EMIT)
#undef MU_EMIT

    default: return 0;
  }
}

#endif /* MU_STATOR_I */
