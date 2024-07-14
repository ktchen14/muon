#ifndef MU_STATOR_MEMBER_TYPE_I
#define MU_STATOR_MEMBER_TYPE_I

#include <muon/stator/member_type.h>  // IWYU pragma: export

#include "abstract_type.h"

#include <stddef.h>

__attribute__((nonnull, pure))
static inline const mu_type_t *member_type_at(
    const mu_member_type_t *type, size_t i) {
  return i == 0 ? type->matter : NULL;
}

const mu_member_type_t *member_type_reduce(
    const mu_member_type_t *type, inductor_t *inductor)
  __attribute__((nonnull));

#endif /* MU_STATOR_MEMBER_TYPE_I */
