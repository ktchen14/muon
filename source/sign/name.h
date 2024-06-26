#ifndef MU_SIGN_NAME_I
#define MU_SIGN_NAME_I

#include <muon/sign/name.h>  // IWYU pragma: export

#include "common.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline size_t name_sign_size(const mu_name_sign_t *sign) {
  return sizeof(mu_name_sign_t);
}

#endif /* MU_SIGN_NAME_I */
