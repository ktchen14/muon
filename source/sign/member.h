#ifndef MU_SIGN_MEMBER_I
#define MU_SIGN_MEMBER_I

#include <muon/sign/member.h>  // IWYU pragma: export

#include "common.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline size_t member_sign_size(const mu_member_sign_t *sign) {
  return sizeof(mu_member_sign_t);
}

#endif /* MU_SIGN_MEMBER_I */
