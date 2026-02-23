#ifndef MU_INDUCTOR_CORE_H
#define MU_INDUCTOR_CORE_H

#include "common.h"

#include "../engine.h"

#include <stddef.h>

typedef enum {
  MUON_BOOLEAN_CORE,
  MUON_CUSTOM_CORE,
  MUON_INTEGER_CORE,
  MUON_LAMBDA_CORE,
  MUON_RECORD_CORE,
  MUON_VECTOR_CORE,
} MuonCoreTag;

typedef enum {
  MUON_COVARIANCE     = 1 << 0,
  MUON_CONTRAVARIANCE = 1 << 1,
  MUON_INVARIANCE     = MUON_COVARIANCE | MUON_CONTRAVARIANCE,
} MuonVariance;

typedef struct {
  MuonName *name;
  MuonVariance variance : 2;
} MuonCoreMember;

typedef const struct MuonCore {
  union { MuonCoreTag kind, tag; };
  const induce_t *induce;
  MuonName *name;
  size_t argc;
  MuonCoreMember argv[/* argc */];
} MuonCore;

typedef const struct MuonExpr MuonExpr;
typedef struct {
  MuonCore *source;
  MuonCore *target;
  MuonExpr *expr;
} mu_instance_t;

MuonCore *mu_simple_core(induce_t *induce, MuonName *name);

const mu_instance_t *mu_instance(
    MuonCore *source, MuonCore *target, MuonExpr *expr)
  MUON_HINT_SUFFIX(malloc, nonnull);

/// Emit debugging information on the abstract @a core to the debug stream
void mu_core_debug(MuonCore *core)
  MUON_HINT_SUFFIX(nonnull);

/// Emit debugging information on the abstract @a instance to the debug stream
void mu_instance_debug(const mu_instance_t *instance)
  MUON_HINT_SUFFIX(nonnull);

#endif /* MU_INDUCTOR_CORE_H */
