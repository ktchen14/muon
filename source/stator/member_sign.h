#ifndef MU_STATOR_MEMBER_SIGN_I
#define MU_STATOR_MEMBER_SIGN_I

#include <muon/stator/member_sign.h>  // IWYU pragma: export

#include "abstract_node.h"

#include <stddef.h>

__attribute__((nonnull, pure))
static inline const mu_node_t *member_sign_at(
    const mu_member_sign_t *sign, size_t i) {
  return i == 0 ? &sign->matter->as_node : NULL;
}

#endif /* MU_STATOR_MEMBER_SIGN_I */
