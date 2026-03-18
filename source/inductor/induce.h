#ifndef MU_INDUCTOR_INDUCE_I
#define MU_INDUCTOR_INDUCE_I

#include "common.h"

#include "../engine.h"

#include <assert.h>
#include <stddef.h>

Rule *type_restrain(
    Inductor *inductor, MuonType *source, MuonType *target, MuonNode *reason)
  MUON_HINT_SUFFIX(nonnull(1, 2, 3));

Rule *type_assess(
    Inductor *inductor, MuonType *restrict source, MuonType *restrict target);

#endif /* MU_INDUCTOR_INDUCE_I */
