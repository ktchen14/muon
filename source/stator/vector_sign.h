#ifndef MU_STATOR_VECTOR_SIGN_I
#define MU_STATOR_VECTOR_SIGN_I

#include <muon/stator/vector_sign.h>  // IWYU pragma: export

#include "abstract_node.h"

#include <stddef.h>

/// Return the <em>i</em>th node in the vector @a sign
__attribute__((nonnull, pure))
static inline const mu_node_t *vector_sign_at(
    const mu_vector_sign_t *sign, size_t i) {
  return i == 0 ? &sign->matter->as_node : NULL;
}

#endif /* MU_STATOR_VECTOR_SIGN_I */
