#include "core.h"

#include "common.h"
#include "name.h"
#include "stator.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

/// @internal Return the mutable engine of the @a core
static inline MuonEngine *unlock_engine(struct MuonCore *core) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
  return (MuonEngine *) core->engine;
#pragma GCC diagnostic pop
}

MuonCore *mu_simple_core(MuonEngine *engine, MuonName *name) {
  struct MuonCore *core;
  if ((core = malloc(sizeof(MuonCore))) == NULL)
    return NULL;
  *core = (MuonCore) {.tag = MUON_CUSTOM_CORE, .engine = engine, .name = name};
  return core;
}

MuonCore *muon_record_core(
    MuonEngine *engine, size_t argc, MuonCoreMember argv[const /* argv */]) {
  struct MuonCore *result;
  if ((result = record_core_allocate(engine, argc)) == NULL)
    return NULL;
  for (size_t i = 0; i < argc; i++)
    result->argv[i] = argv[i];
  return record_core_activate(result);
}

struct MuonCore *record_core_allocate(MuonEngine *engine, size_t argc) {
  size_t size;
  if (rare((size = struct_size(MuonCore, argv, argc)) == 0))
    return NULL;

  struct MuonCore *allocation;
  if ((allocation = engine_allocate(engine, size)) == NULL)
    return NULL;
  *allocation = (MuonCore) {
    .tag = MUON_RECORD_CORE, .engine = engine, .argc = argc
  };
  return allocation;
}

MuonCore *record_core_activate(struct MuonCore *allocation) {
  assert(allocation->tag == MUON_RECORD_CORE);

  MuonEngine *opaque = unlock_engine(allocation);
  for (size_t i = 0; i < allocation->argc; i++) {
    MuonName *name = allocation->argv[i].name;
    assert(name != NULL && name->engine == opaque);
    assert(allocation->argv[i].i == i);
    assert(allocation->argv[i].variance == 0);
  }

  // Ensure that each member is sorted after the previous one
  for (size_t i = 1; i < allocation->argc; i++)
    assert(
        name_cmp(allocation->argv[i].name, allocation->argv[i - 1].name) > 0);

  Engine *engine = as_engine(opaque);

  for (size_t i = 0; i < engine->core_length; i++) {
    MuonCore *core = engine->core[i];

    if (core->tag != MUON_RECORD_CORE || core->argc != allocation->argc)
      continue;

    for (size_t j = 0; j < allocation->argc; j++) {
      if (core->argv[j].name != allocation->argv[j].name)
        goto next;
    }

    return free(allocation), core;

  next:
  }

  return engine->core[engine->core_length++] = allocation;
}

void muon_core_debug(MuonCore *core) {
  static const char *const VARIANCE[] = {"+", "-"};

  switch (core->tag) {
    case MUON_BOOLEAN_CORE:
      debug(PRIsKIND, DEBUG_CORE_KIND("Boolean"));
      return;
    case MUON_CUSTOM_CORE:
      debug(PRIsKIND, DEBUG_CORE_KIND(DEBUG_NAME(core->name)));
      break;
    case MUON_INTEGER_CORE:
      debug(PRIsKIND, DEBUG_CORE_KIND("Integer"));
      return;
    case MUON_LAMBDA_CORE:
      debug(PRIsKIND, DEBUG_CORE_KIND("λ"));
      break;
    case MUON_RECORD_CORE:
    case MUON_VECTOR_CORE:
      break;
  }

  debug("%c", "(["[core->tag == MUON_VECTOR_CORE]);
  for (size_t i = 0; i < core->argc; i++) {
    if (i > 0)
      debug(", ");
    if (core->argv[i].name != NULL)
      debug(PRIsNAME ": ", DEBUG_NAME(core->argv[i].name));
    debug("%s", VARIANCE[core->argv[i].variance]);
  }
  debug("%c", ")]"[core->tag == MUON_VECTOR_CORE]);
}
