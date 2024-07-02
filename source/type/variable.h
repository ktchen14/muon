#ifndef MU_TYPE_VARIABLE_I
#define MU_TYPE_VARIABLE_I

#include <muon/type/variable.h>  // IWYU pragma: export

#include "common.h"

#include <assert.h>
#include <stddef.h>

__attribute__((const, nonnull))
static inline size_t variable_type_size(const mu_variable_type_t *type) {
  return sizeof(mu_variable_type_t);
}

__attribute__((nonnull, pure))
static inline const mu_variable_type_t *variable_type_reduce(
    const mu_variable_type_t *type,
    mu_engine_t *engine,
    const mu_type_t *const equation[]) {
  assert(0);
}

__attribute__((const, nonnull))
static inline const mu_type_t *variable_type_at(
    const mu_variable_type_t *type, size_t i) {
  return NULL;
}

#endif /* MU_TYPE_VARIABLE_I */
