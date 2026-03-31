#ifndef MUON_ENGINE_INSTANCE_H
#define MUON_ENGINE_INSTANCE_H

#include "common.h"
#include "type.h"

#include <stddef.h>

typedef const struct MuonInstance {
  const MuonEngine *engine;
  size_t id;
  MuonSchemeType *scheme;
  MuonImplicitType *argv[];
} MuonInstance;

MuonInstance *muon_instance(MuonEngine *engine, MuonSchemeType *scheme)
  MUON_HINT_SUFFIX(nonnull);

#endif /* MUON_ENGINE_INSTANCE_H */
