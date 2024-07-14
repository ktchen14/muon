#ifndef MU_STATOR_NAME_SIGN_I
#define MU_STATOR_NAME_SIGN_I

#include <muon/stator/name_sign.h>  // IWYU pragma: export

#include "abstract_node.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline const mu_node_t *name_sign_at(
    const mu_name_sign_t *sign, size_t i) {
  return NULL;
}

#endif /* MU_STATOR_NAME_SIGN_I */
