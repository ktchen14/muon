#ifndef MU_SIGN_INTEGER_I
#define MU_SIGN_INTEGER_I

#include <muon/sign/integer.h>  // IWYU pragma: export

#include "common.h"
#include "../type.h"

__attribute__((const, nonnull))
static inline size_t integer_sign_size(const mu_integer_sign_t *sign) {
  return sizeof(mu_integer_sign_t);
}

#endif /* MU_SIGN_INTEGER_I */
