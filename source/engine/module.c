#include "module.h"

#include "common.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>

/// @internal Return the mutable engine of the @a module
static inline MuonEngine *unlock_engine(struct MuonModule *module) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
  return (MuonEngine *) module->engine;
#pragma GCC diagnostic pop
}

MuonExport *muon_export(MuonEngine *engine, MuonName *name, MuonType *type) {
  assert(name->as_stator.engine == engine);
  assert(type->as_stator.engine == engine);

  struct MuonExport *result;
  if ((result = engine_allocate(engine, sizeof(MuonExport))) == NULL)
    return NULL;
  *result = (MuonExport) {.engine = engine, .name = name, .type = type};
  return result;
}

MuonModule *muon_module(
    MuonEngine *engine, size_t argc, MuonExport *argv[/* argc */]) {
  struct MuonModule *allocation;
  if ((allocation = module_allocate(engine, argc)) == NULL)
    return NULL;
  for (size_t i = 0; i < argc; i++)
    allocation->argv[i] = argv[i];
  return module_activate(allocation);
}

void muon_export_debug(MuonExport *export) {
  debug("  ");
  muon_name_debug(export->name);
  debug(": ");
  muon_type_debug(export->type);
  debug("\n");
}

void muon_module_debug(MuonModule *module) {
  for (size_t i = 0; i < module->argc; i++)
    muon_export_debug(module->argv[i]);
}

struct MuonModule *module_allocate(MuonEngine *engine, size_t argc) {
  size_t size = argc;
  if (struct_size_overflow(MuonModule, argv, &size))
    return errno = ENOMEM, NULL;

  struct MuonModule *result;
  if ((result = engine_allocate(engine, size)) == NULL)
    return NULL;
  *result = (MuonModule) {.engine = engine, .argc = argc};
  return result;
}

MuonModule *module_activate(struct MuonModule *module) {
  MuonEngine *engine = unlock_engine(module);

  for (size_t i = 0; i < module->argc; i++) {
    assert(module->argv[i] != NULL);
    assert(module->argv[i]->engine == engine);
  }
  return module;
}
