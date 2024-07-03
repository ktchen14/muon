#ifndef MU_SIGN_MEMBER_I
#define MU_SIGN_MEMBER_I

#include <muon/node/member_sign.h>  // IWYU pragma: export

#include "common.h"

#include <stddef.h>

__attribute__((nonnull, pure))
static inline const mu_node_t *member_sign_at(
    const mu_member_sign_t *sign, size_t i) {
  return i == 0 ? &sign->matter->as_node : NULL;
}

#endif /* MU_SIGN_MEMBER_I */
