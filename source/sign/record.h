#ifndef MU_SIGN_RECORD_I
#define MU_SIGN_RECORD_I

#include <muon/sign/record.h>  // IWYU pragma: export

#include "common.h"

#include <stddef.h>

__attribute__((nonnull, pure))
static inline size_t record_sign_size(const mu_record_sign_t *sign) {
  return extant_size(mu_record_sign_t, argv, sign->argc);
}

#endif /* MU_SIGN_RECORD_I */
