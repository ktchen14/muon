#ifndef MU_TYPE_INTEGER_I
#define MU_TYPE_INTEGER_I

#include <muon/type/integer.h>  // IWYU pragma: export

#include "common.h"

__attribute__((const, nonnull))
static inline size_t integer_type_size(const mu_integer_type_t *type) {
  return sizeof(mu_integer_type_t);
}

#endif /* MU_TYPE_INTEGER_I */
