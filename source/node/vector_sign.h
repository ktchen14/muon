#ifndef MU_SIGN_VECTOR_I
#define MU_SIGN_VECTOR_I

#include <muon/node/vector_sign.h>  // IWYU pragma: export

#include "common.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline size_t vector_sign_size(const mu_vector_sign_t *sign) {
  return sizeof(mu_vector_sign_t);
}

/// Return the <em>i</em>th node in the vector @a sign
__attribute__((nonnull, pure))
static inline const mu_node_t *vector_sign_at(
    const mu_vector_sign_t *sign, size_t i) {
  return i == 0 ? &sign->matter->as_node : NULL;
}

#endif /* MU_SIGN_VECTOR_I */
