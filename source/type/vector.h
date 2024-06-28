#ifndef MU_TYPE_VECTOR_I
#define MU_TYPE_VECTOR_I

#include <muon/type/vector.h>  // IWYU pragma: export

#include "common.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline size_t vector_type_size(const mu_vector_type_t *type) {
  return sizeof(mu_vector_type_t);
}

#endif /* MU_TYPE_VECTOR_I */
