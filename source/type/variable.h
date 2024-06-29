#ifndef MU_TYPE_VARIABLE_I
#define MU_TYPE_VARIABLE_I

#include <muon/type/variable.h>  // IWYU pragma: export

#include "common.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline size_t variable_type_size(const mu_variable_type_t *type) {
  return sizeof(mu_variable_type_t);
}

#endif /* MU_TYPE_VARIABLE_I */
