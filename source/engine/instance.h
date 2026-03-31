#ifndef MUON_ENGINE_INSTANCE_I
#define MUON_ENGINE_INSTANCE_I

#include <muon/engine/instance.h>

#include "common.h"

#include <stddef.h>

struct MuonInstance *instance_allocate(
    MuonEngine *engine, MuonSchemeType *scheme)
  MUON_HINT_SUFFIX(nonnull);

MuonInstance *instance_activate(struct MuonInstance *instance)
  MUON_HINT_SUFFIX(nonnull);

#endif /* MUON_ENGINE_INSTANCE_I */
