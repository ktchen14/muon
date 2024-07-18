#ifndef MU_STATOR_LAMBDA_TYPE_I
#define MU_STATOR_LAMBDA_TYPE_I

#include <muon/stator/lambda_type.h>  // IWYU pragma: export

#include "abstract_type.h"

#include <stddef.h>

__attribute__((nonnull, pure))
static inline const mu_type_t *lambda_type_at(
    const mu_lambda_type_t *type, size_t i) {
  switch (i) {
    case 0: return type->argument;
    case 1: return type->output;
    default: return NULL;
  }
}

const mu_lambda_type_t *lambda_type_import(
    const mu_lambda_type_t *type, const import_t *import)
  __attribute__((nonnull));

#endif /* MU_STATOR_LAMBDA_TYPE_I */
