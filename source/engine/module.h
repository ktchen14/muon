#ifndef MUON_ENGINE_MODULE_I
#define MUON_ENGINE_MODULE_I

#include <muon/engine/module.h>

#include "common.h"

#include <stddef.h>

struct MuonModule *module_allocate(MuonEngine *engine, size_t argc)
  MUON_HINT_SUFFIX(nonnull);

MuonModule *module_activate(struct MuonModule *module)
  MUON_HINT_SUFFIX(nonnull);

#endif /* MUON_ENGINE_MODULE_I */
