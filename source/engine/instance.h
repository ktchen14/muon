#ifndef MUON_ENGINE_INSTANCE_I
#define MUON_ENGINE_INSTANCE_I

#include <muon/engine/instance.h>

#include "common.h"

struct MuonInstance *instance_allocate(MuonEngine *engine)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonInstance *instance_activate(
    struct MuonInstance *instance, MuonSchemeType *scheme)
  MUON_HINT_SUFFIX(nonnull);

#endif /* MUON_ENGINE_INSTANCE_I */
