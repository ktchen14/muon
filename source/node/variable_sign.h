#ifndef MU_SIGN_VARIABLE_I
#define MU_SIGN_VARIABLE_I

#include <muon/node/variable_sign.h>  // IWYU pragma: export

#include "common.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline size_t variable_sign_size(const mu_variable_sign_t *sign) {
  return sizeof(mu_variable_sign_t);
}

__attribute__((const, nonnull))
static inline const mu_node_t *variable_sign_at(
    const mu_variable_sign_t *sign, size_t i) {
  return NULL;
}

#endif /* MU_SIGN_VARIABLE_I */
