#ifndef MU_SIGN_RECORD_I
#define MU_SIGN_RECORD_I

#include <muon/node/record_sign.h>  // IWYU pragma: export

#include "common.h"

#include <stddef.h>

__attribute__((nonnull, pure))
static inline size_t record_sign_size(const mu_record_sign_t *sign) {
  return extant_size(mu_record_sign_t, argv, sign->argc);
}

__attribute__((nonnull, pure))
static inline const mu_node_t *record_sign_at(
    const mu_record_sign_t *sign, size_t i) {
  return i < sign->argc ? &sign->argv[i]->as_node : NULL;
}

#endif /* MU_SIGN_RECORD_I */
