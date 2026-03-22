#include "core.h"

#include "common.h"
#include "name.h"
#include "stator.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>

/// @internal Return the mutable engine of the @a core
static inline MuonEngine *unlock_engine(struct MuonCore *core) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
  return (MuonEngine *) core->engine;
#pragma GCC diagnostic pop
}

static inline Hash core_hash(MuonCoreTag tag, Hash hash) {
  tag |= CORE_PREFIX << 5;
  return hash >> 8 | (Hash) tag << sizeof(Hash) * CHAR_BIT - 8;
}

MuonCustomCore *mu_simple_core(MuonEngine *engine, MuonName *name) {
  struct MuonCustomCore *result;
  if ((result = malloc(sizeof(MuonCustomCore))) == NULL)
    return NULL;
  *result = (MuonCustomCore) {
    .as_core = {.tag = MUON_CUSTOM_CORE, .engine = engine}, .name = name
  };
  return result;
}

MuonRecordCore *muon_record_core(
    MuonEngine *engine, size_t argc, MuonName *const argv[/* argv */]) {
  struct MuonRecordCore *result;
  if ((result = record_core_allocate(engine, argc)) == NULL)
    return NULL;
  for (size_t i = 0; i < argc; i++)
    result->argv[i] = argv[i];
  return record_core_activate(result);
}

struct MuonRecordCore *record_core_allocate(MuonEngine *engine, size_t argc) {
  size_t size = argc;
  if (struct_size_overflow(MuonRecordCore, argv, &size))
    return NULL;

  struct MuonRecordCore *allocation;
  if ((allocation = engine_allocate(engine, size)) == NULL)
    return NULL;
  *allocation = (MuonRecordCore) {
    .as_core = {.tag = MUON_RECORD_CORE, .engine = engine}, .argc = argc
  };
  return allocation;
}

MuonRecordCore *record_core_activate(struct MuonRecordCore *core) {
  MuonEngine *engine = unlock_engine(&core->as_core);

  for (size_t i = 0; i < core->argc; i++) {
    assert(core->argv[i] != NULL);
    assert(core->argv[i]->engine == engine);
  }

  for (size_t i = 1; i < core->argc; i++)
    assert(name_cmp(core->argv[i], core->argv[i - 1]) > 0);

  Hash hash = HASH_ZERO;
  for (size_t i = 0; i < core->argc; i++)
    hash = hash_extend(hash, core->argv[i]);
  hash = core_hash(MUON_RECORD_CORE, hash);

  MuonRecordCore *next;
  size_t i = 0;
  for (; (next = stator_search(engine, hash, &i)) != NULL; i++) {
    if (next->argc != core->argc)
      continue;

    for (size_t i = 0; i < core->argc; i++) {
      if (next->argv[i] != core->argv[i])
        goto next;
    }

    return free(core), next;
  next:
  }

  return stator_insert(engine, core, hash, i);
}

// TODO: remove this
/// Literal printf specifier for a kind
#define PRIsKIND "%s%s%s"

/// Used with PRIsKIND to emit the @a text as a core kind
#define DEBUG_CORE_KIND(text) "", (text), ""

void muon_core_debug(MuonCore *core) {
  static const char *const VARIANCE[] = {"+", "-"};

  switch ON_ABSTRACT_CORE(core) {
    case MUON_BOOLEAN_CORE:
      debug(PRIsKIND, DEBUG_CORE_KIND("Boolean"));
      break;

    case IS_CONCRETE_CORE(MuonCustomCore *custom_core)
      debug(PRIsKIND, DEBUG_CORE_KIND(DEBUG_NAME(custom_core->name)));

      if (custom_core->argc == 0)
        break;

      debug("(");
      for (size_t i = 0; i < custom_core->argc; i++) {
        MuonCoreMember member = custom_core->argv[i];
        if (i > 0)
          debug(", ");
        if (member.name != NULL)
          debug(PRIsNAME ": ", DEBUG_NAME(member.name));
        debug("%s", VARIANCE[member.variance]);
      }
      debug(")");
      break;

    case MUON_INTEGER_CORE:
      debug(PRIsKIND, DEBUG_CORE_KIND("Integer"));
      break;

    case MUON_LAMBDA_CORE:
      debug(PRIsKIND, DEBUG_CORE_KIND("(→)"));
      break;

    case IS_CONCRETE_CORE(MuonRecordCore *record_core)
      debug("(");
      for (size_t i = 0; i < record_core->argc; i++) {
        if (i > 0)
          debug(", ");
        if (record_core->argv[i] != NULL)
          debug(PRIsNAME ": ", DEBUG_NAME(record_core->argv[i]));
        debug("+");
      }
      debug(")");
      break;

    case MUON_VECTOR_CORE:
      debug(PRIsKIND, DEBUG_CORE_KIND("[]"));
      break;
  }
}
