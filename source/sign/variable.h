#ifndef MU_SIGN_VARIABLE_I
#define MU_SIGN_VARIABLE_I

#include <muon/sign/variable.h>  // IWYU pragma: export

#include "common.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline size_t variable_sign_size(const mu_variable_sign_t *sign) {
  return sizeof(mu_variable_sign_t);
}

#endif /* MU_SIGN_VARIABLE_I */
