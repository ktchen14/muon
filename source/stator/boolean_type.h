#ifndef MU_STATOR_BOOLEAN_TYPE_I
#define MU_STATOR_BOOLEAN_TYPE_I

#include <muon/stator/boolean_type.h>  // IWYU pragma: export

#include "abstract_type.h"

#include <assert.h>
#include <stddef.h>

__attribute__((const, nonnull))
static inline size_t boolean_type_size(const mu_boolean_type_t *type) {
  return sizeof(mu_boolean_type_t);
}

__attribute__((const, nonnull))
static inline const mu_boolean_type_t *boolean_type_reduce(
    const mu_boolean_type_t *type, const inductor_t *inductor) {
  return type;
}

__attribute__((const, nonnull))
static inline const mu_type_t *boolean_type_at(
    const mu_boolean_type_t *type, size_t i) {
  return NULL;
}

#endif /* MU_STATOR_BOOLEAN_TYPE_I */
