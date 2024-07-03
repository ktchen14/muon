#ifndef MU_SIGN_INTEGER_I
#define MU_SIGN_INTEGER_I

#include <muon/node/integer_sign.h>  // IWYU pragma: export

#include "common.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline size_t integer_sign_size(const mu_integer_sign_t *sign) {
  return sizeof(mu_integer_sign_t);
}

__attribute__((const, nonnull))
static inline const mu_node_t *integer_sign_at(
    const mu_integer_sign_t *sign, size_t i) {
  return NULL;
}

#endif /* MU_SIGN_INTEGER_I */
