#ifndef MU_SIGN_VECTOR_I
#define MU_SIGN_VECTOR_I

#include <muon/sign/vector.h>  // IWYU pragma: export

#include "common.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline size_t vector_sign_size(const mu_vector_sign_t *sign) {
  return sizeof(mu_vector_sign_t);
}

#endif /* MU_SIGN_VECTOR_I */
