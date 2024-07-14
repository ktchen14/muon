#ifndef MU_STATOR_VECTOR_TYPE_I
#define MU_STATOR_VECTOR_TYPE_I

#include <muon/stator/vector_type.h>  // IWYU pragma: export

#include "abstract_type.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline size_t vector_type_size(const mu_vector_type_t *type) {
  return sizeof(mu_vector_type_t);
}

const mu_vector_type_t *vector_type_reduce(
    const mu_vector_type_t *type, inductor_t *inductor)
  __attribute__((nonnull));

__attribute__((nonnull, pure))
static inline const mu_type_t *vector_type_at(
    const mu_vector_type_t *type, size_t i) {
  return i == 0 ? type->matter : NULL;
}

#endif /* MU_STATOR_VECTOR_TYPE_I */
