#ifndef MU_TYPE_VECTOR_I
#define MU_TYPE_VECTOR_I

#include <muon/type/vector.h>  // IWYU pragma: export

#include "common.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline size_t vector_type_size(const mu_vector_type_t *type) {
  return sizeof(mu_vector_type_t);
}

__attribute__((nonnull))
const mu_vector_type_t *vector_type_reduce(
    const mu_vector_type_t *type,
    mu_engine_t *engine,
    inductor_t *inductor);

__attribute__((nonnull, pure))
static inline const mu_type_t *vector_type_at(
    const mu_vector_type_t *type, size_t i) {
  return i == 0 ? type->matter : NULL;
}

#endif /* MU_TYPE_VECTOR_I */
