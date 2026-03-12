#ifndef MUON_ENGINE_H
#define MUON_ENGINE_H

#include "engine/common.h" // IWYU pragma: export
#include "engine/core.h"   // IWYU pragma: export
#include "engine/module.h" // IWYU pragma: export
#include "engine/name.h"   // IWYU pragma: export
#include "engine/node.h"   // IWYU pragma: export
#include "engine/stator.h" // IWYU pragma: export
#include "engine/type.h"   // IWYU pragma: export

MuonEngine *muon_engine_initialize(MuonEngine *engine)
  MUON_HINT_SUFFIX(nonnull);

#endif /* MUON_ENGINE_H */
