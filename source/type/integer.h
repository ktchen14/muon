#ifndef MU_TYPE_INTEGER_I
#define MU_TYPE_INTEGER_I

#include <muon/type/integer.h>  // IWYU pragma: export

#include "common.h"

#include <assert.h>
#include <stddef.h>

__attribute__((const, nonnull))
static inline size_t integer_type_size(const mu_integer_type_t *type) {
  return sizeof(mu_integer_type_t);
}

__attribute__((const, nonnull))
static inline const mu_integer_type_t *integer_type_reduce(
    const mu_integer_type_t *type,
    mu_engine_t *engine,
    const inductor_t *inductor) {
  assert(engine == type->as_stator.engine);
  return type;
}

__attribute__((const, nonnull))
static inline const mu_type_t *integer_type_at(
    const mu_integer_type_t *type, size_t i) {
  return NULL;
}

#endif /* MU_TYPE_INTEGER_I */
