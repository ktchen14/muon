#ifndef MU_STATOR_INTEGER_TYPE_I
#define MU_STATOR_INTEGER_TYPE_I

#include <muon/stator/integer_type.h>  // IWYU pragma: export

#include "abstract_type.h"

#include <assert.h>
#include <stddef.h>

__attribute__((const, nonnull))
static inline size_t integer_type_size(const mu_integer_type_t *type) {
  return sizeof(mu_integer_type_t);
}

__attribute__((const, nonnull))
static inline const mu_integer_type_t *integer_type_reduce(
    const mu_integer_type_t *type, const inductor_t *inductor) {
  return type;
}

__attribute__((const, nonnull))
static inline const mu_type_t *integer_type_at(
    const mu_integer_type_t *type, size_t i) {
  return NULL;
}

#endif /* MU_STATOR_INTEGER_TYPE_I */
