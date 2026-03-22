#ifndef MUON_ENGINE_MODULE_H
#define MUON_ENGINE_MODULE_H

#include "common.h"
#include "name.h"
#include "type.h"

#include <stddef.h>

typedef const struct MuonExport {
  const MuonEngine *engine;

  MuonName *name;
  MuonType *type;
} MuonExport;

typedef const struct MuonModule {
  const MuonEngine *engine;

  size_t argc;
  MuonExport *argv[] MUON_HINT(counted_by(argc));
} MuonModule;

MuonExport *muon_export(MuonEngine *engine, MuonName *name, MuonType *type)
  MUON_HINT_SUFFIX(nonnull);

MuonModule *muon_module(
    MuonEngine *engine, size_t argc, MuonExport *argv[/* argc */])
  MUON_HINT_SUFFIX(nonnull);

void muon_export_debug(MuonExport *export);
void muon_module_debug(MuonModule *module);

#endif /* MUON_ENGINE_MODULE_H */
