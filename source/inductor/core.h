#ifndef MU_INDUCTOR_CORE_I
#define MU_INDUCTOR_CORE_I

#include "../stator/name.h"

#include <assert.h>
#include <stddef.h>

typedef enum {
  MU_BOOLEAN_CORE,
  MU_INTEGER_CORE,
  MU_LAMBDA_CORE,
  MU_VECTOR_CORE,
  MU_RECORD_CORE,
  MU_CUSTOM_CORE,
} mu_core_kind_t;

typedef enum {
  MU_COVARIANCE,
  MU_CONTRAVARIANCE,
  MU_INVARIANCE,
} mu_variance_t;

typedef struct {
  const mu_name_t *name;
  mu_variance_t variance;
} mu_core_member_t;

typedef struct induce_t induce_t;

typedef struct {
  mu_core_kind_t kind;
  const induce_t *induce;
  const mu_name_t *name;
  size_t argc;
  mu_core_member_t argv[/* argc */];
} mu_core_t;

typedef struct {
  const mu_core_t *target;
  const mu_core_t *source;
  size_t argv[/* target->argc */];
} record_instance_t;

const mu_core_t *mu_simple_core(induce_t *induce, const mu_name_t *name);

const mu_core_t *single_record_core(induce_t *induce, const mu_name_t *name)
  __attribute__((malloc, nonnull));

mu_core_t *record_core_allocate(induce_t *induce, size_t argc)
  __attribute__((malloc, nonnull));

const mu_core_t *record_core_activate(mu_core_t *core)
  __attribute__((nonnull));

const record_instance_t *get_record_instance(
    induce_t *induce, const mu_core_t *source, const mu_core_t *target)
  __attribute__((nonnull));

/// Compare the core member @a a to the core member @a b
__attribute__((nonnull, pure))
static inline int core_member_cmp(const void *a, const void *b) {
  const mu_core_member_t *ra = a, *rb = b;
  assert(ra->name != NULL && rb->name != NULL);
  return name_cmp(ra->name, rb->name);
}

void mu_core_debug(const mu_core_t *core) __attribute__((nonnull));

#endif /* MU_INDUCTOR_CORE_I */
