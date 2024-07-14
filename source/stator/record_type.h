#ifndef MU_STATOR_RECORD_TYPE_I
#define MU_STATOR_RECORD_TYPE_I

#include <muon/stator/record_type.h>  // IWYU pragma: export

#include "abstract_type.h"

#include <stddef.h>

const mu_record_type_t *record_type_reduce(
    const mu_record_type_t *type, inductor_t *inductor)
  __attribute__((nonnull));

__attribute__((nonnull, pure))
static inline const mu_type_t *record_type_at(
    const mu_record_type_t *type, size_t i) {
  return i < type->argc ? type->argv[i] : NULL;
}

#endif /* MU_STATOR_RECORD_TYPE_I */
