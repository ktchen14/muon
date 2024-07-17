#ifndef MU_STATOR_RECORD_TYPE_I
#define MU_STATOR_RECORD_TYPE_I

#include <muon/stator/record_type.h>  // IWYU pragma: export

#include "abstract_type.h"
#include "name.h"

#include <assert.h>
#include <stddef.h>

mu_record_type_t *record_type_allocate(mu_engine_t *engine, size_t argc)
  __attribute__((malloc, nonnull));

const mu_record_type_t *record_type_activate(mu_record_type_t *type)
  __attribute__((nonnull, returns_nonnull));

__attribute__((nonnull, pure))
static inline const mu_type_t *record_type_at(
    const mu_record_type_t *type, size_t i) {
  return i < type->argc ? type->argv[i].type : NULL;
}

const mu_record_type_t *record_type_import(
    const mu_record_type_t *type, const import_t *import)
  __attribute__((nonnull));

/// Compare the type member @a a to the type member @a b
__attribute__((nonnull, pure))
static inline int type_member_cmp(const void *a, const void *b) {
  const mu_type_member_t *ra = a, *rb = b;
  assert(ra->name != NULL && rb->name != NULL);
  return name_cmp(ra->name, rb->name);
}

#endif /* MU_STATOR_RECORD_TYPE_I */
