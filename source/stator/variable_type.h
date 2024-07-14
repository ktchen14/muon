#ifndef MU_STATOR_VARIABLE_TYPE_I
#define MU_STATOR_VARIABLE_TYPE_I

#include <muon/stator/variable_type.h>  // IWYU pragma: export

#include "abstract_type.h"

#include <assert.h>
#include <stddef.h>

__attribute__((const, nonnull))
static inline const mu_type_t *variable_type_at(
    const mu_variable_type_t *type, size_t i) {
  return NULL;
}

__attribute__((nonnull, pure))
static inline const mu_variable_type_t *variable_type_reduce(
    const mu_variable_type_t *type, const inductor_t *inductor_t) {
  return type;
}

#endif /* MU_STATOR_VARIABLE_TYPE_I */
