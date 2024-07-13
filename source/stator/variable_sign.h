#ifndef MU_STATOR_VARIABLE_SIGN_I
#define MU_STATOR_VARIABLE_SIGN_I

#include <muon/stator/variable_sign.h>  // IWYU pragma: export

#include "node.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline const mu_node_t *variable_sign_at(
    const mu_variable_sign_t *sign, size_t i) {
  return NULL;
}

#endif /* MU_STATOR_VARIABLE_SIGN_I */
