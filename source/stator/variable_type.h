#ifndef MU_STATOR_VARIABLE_TYPE_I
#define MU_STATOR_VARIABLE_TYPE_I

#include <muon/stator/variable_type.h>  // IWYU pragma: export

#include "abstract_type.h"
#include "test.h"

#include <assert.h>
#include <stddef.h>

mu_variable_type_t *variable_type_allocate(mu_engine_t *engine, size_t argc)
  __attribute__((malloc, nonnull));

const mu_variable_type_t *variable_type_activate(mu_variable_type_t *type)
  __attribute__((nonnull, returns_nonnull));

__attribute__((const, nonnull))
static inline const mu_type_t *variable_type_at(
    const mu_variable_type_t *type, size_t i) {
  if (i >= type->argc)
    return NULL;

  const mu_test_t *test = type->argv[i];

  const mu_member_test_t *member_test;
  if ((member_test = mu_test_cast(test, member_test)) != NULL)
    return member_test->type;
  return NULL;
}

__attribute__((nonnull))
static inline const mu_variable_type_t *variable_type_import(
    const mu_variable_type_t *type, const import_t *import) {
  if (import->engine == type->as_stator.engine)
    return type;
  return mu_variable_type(import->engine, 0, NULL);
}

__attribute__((nonnull, pure))
static inline const mu_test_t *variable_type_test_at(
    const mu_variable_type_t *type, size_t i) {
  return i < type->argc ? type->argv[i] : NULL;
}

#endif /* MU_STATOR_VARIABLE_TYPE_I */
