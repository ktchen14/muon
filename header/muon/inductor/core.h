#ifndef MU_INDUCTOR_CORE_H
#define MU_INDUCTOR_CORE_H

#include "../engine/name.h"

#include <stddef.h>

typedef enum {
  MU_BOOLEAN_CORE,
  MU_CUSTOM_CORE,
  MU_INTEGER_CORE,
  MU_LAMBDA_CORE,
  MU_RECORD_CORE,
  MU_VECTOR_CORE,
} mu_core_kind_t;

typedef enum {
  MU_COVARIANCE,
  MU_CONTRAVARIANCE,
  MU_INVARIANCE,
} mu_variance_t;

typedef struct {
  MuonName *name;
  mu_variance_t variance;
} mu_core_member_t;

typedef struct induce_t induce_t;

typedef struct {
  mu_core_kind_t kind;
  const induce_t *induce;
  MuonName *name;
  size_t argc;
  mu_core_member_t argv[/* argc */];
} mu_core_t;

typedef const struct mu_expr_t mu_expr_t;
typedef struct {
  const mu_core_t *source;
  const mu_core_t *target;
  mu_expr_t *expr;
} mu_instance_t;

const mu_core_t *mu_simple_core(induce_t *induce, MuonName *name);

const mu_instance_t *mu_instance(
    const mu_core_t *source, const mu_core_t *target, mu_expr_t *expr)
  __attribute__((malloc, nonnull));

/// Emit debugging information on the abstract @a core to the debug stream
void mu_core_debug(const mu_core_t *core)
  __attribute__((nonnull));

/// Emit debugging information on the abstract @a instance to the debug stream
void mu_instance_debug(const mu_instance_t *instance)
  __attribute__((nonnull));

#endif /* MU_INDUCTOR_CORE_H */
