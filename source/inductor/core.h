#ifndef MU_INDUCTOR_CORE_I
#define MU_INDUCTOR_CORE_I

#include <muon/inductor/core.h>  // IWYU pragma: export

#include "../engine/name.h"

#include <assert.h>
#include <stddef.h>

typedef struct induce_t induce_t;

typedef struct {
  const mu_core_t *target;
  const mu_core_t *source;
  size_t argv[/* target->argc */];
} record_instance_t;

const mu_core_t *single_record_core(induce_t *induce, MuonName *name)
  MUON_HINT_SUFFIX(malloc, nonnull);

mu_core_t *record_core_allocate(induce_t *induce, size_t argc)
  MUON_HINT_SUFFIX(malloc, nonnull);

const mu_core_t *record_core_activate(mu_core_t *core)
  MUON_HINT_SUFFIX(nonnull);

const record_instance_t *get_record_instance(
    induce_t *induce, const mu_core_t *source, const mu_core_t *target)
  MUON_HINT_SUFFIX(nonnull);

/// Compare the core member @a a to the core member @a b
MUON_HINT(nonnull, pure)
static inline int core_member_cmp(const void *a, const void *b) {
  const mu_core_member_t *ra = a, *rb = b;
  assert(ra->name != NULL && rb->name != NULL);
  return name_cmp(ra->name, rb->name);
}

#endif /* MU_INDUCTOR_CORE_I */
