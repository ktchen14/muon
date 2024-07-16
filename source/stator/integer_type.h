#ifndef MU_STATOR_INTEGER_TYPE_I
#define MU_STATOR_INTEGER_TYPE_I

#include <muon/stator/integer_type.h>  // IWYU pragma: export

#include "abstract_type.h"

#include <assert.h>
#include <stddef.h>

__attribute__((const, nonnull))
static inline const mu_type_t *integer_type_at(
    const mu_integer_type_t *type, size_t i) {
  return NULL;
}

__attribute__((nonnull))
static inline const mu_integer_type_t *integer_type_import(
    const mu_integer_type_t *type, const import_t *import) {
  if (import->engine == type->as_stator.engine)
    return type;
  return mu_integer_type(import->engine);
}

#endif /* MU_STATOR_INTEGER_TYPE_I */
