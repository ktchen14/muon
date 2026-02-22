#ifndef MU_INDUCTOR_CORE_H
#define MU_INDUCTOR_CORE_H

#include "../engine/name.h"

#include <stddef.h>

typedef struct induce_t induce_t;

typedef enum {
  MU_BOOLEAN_CORE,
  MU_CUSTOM_CORE,
  MU_INTEGER_CORE,
  MU_LAMBDA_CORE,
  MU_RECORD_CORE,
  MU_VECTOR_CORE,
} mu_core_kind_t;

typedef enum {
  MUON_COVARIANCE     = 1 << 0,
  MUON_CONTRAVARIANCE = 1 << 1,
  MUON_INVARIANCE     = MUON_COVARIANCE | MUON_CONTRAVARIANCE,
} MuonVariance;

typedef struct {
  MuonName *name;
  MuonVariance variance : 2;
} mu_core_member_t;

typedef struct {
  mu_core_kind_t kind;
  const induce_t *induce;
  MuonName *name;
  size_t argc;
  mu_core_member_t argv[/* argc */];
} mu_core_t;

typedef const struct MuonExpr MuonExpr;
typedef struct {
  const mu_core_t *source;
  const mu_core_t *target;
  MuonExpr *expr;
} mu_instance_t;

const mu_core_t *mu_simple_core(induce_t *induce, MuonName *name);

const mu_instance_t *mu_instance(
    const mu_core_t *source, const mu_core_t *target, MuonExpr *expr)
  MUON_HINT_SUFFIX(malloc, nonnull);

/// Emit debugging information on the abstract @a core to the debug stream
void mu_core_debug(const mu_core_t *core)
  MUON_HINT_SUFFIX(nonnull);

/// Emit debugging information on the abstract @a instance to the debug stream
void mu_instance_debug(const mu_instance_t *instance)
  MUON_HINT_SUFFIX(nonnull);

#endif /* MU_INDUCTOR_CORE_H */
