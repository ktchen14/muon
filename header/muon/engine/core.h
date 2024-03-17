#ifndef MUON_ENGINE_CORE_H
#define MUON_ENGINE_CORE_H

#include "common.h"
#include "name.h"
#include "stator.h"

#include <stddef.h>

/// An enumeration over each concrete subtype of MuonCore, e.g.
/// @c MUON_BOOLEAN_CORE
typedef enum {
#define MUON_EMIT(T, l, UPPER) MUON_##UPPER = MUON_##UPPER##_STATOR,
  MUON_EACH_CORE(MUON_EMIT)
#undef MUON_EMIT
} MuonCoreTag;

typedef struct {
  MuonName *name;
  size_t i;
  _Bool variance;
} MuonCoreMember;

typedef const struct MuonCore {
  union { MUON_STATOR_HEADER; MuonCoreTag tag : 8; }; //-

  MuonName *name;
  size_t argc;
  MuonCoreMember argv[/* argc */];
} MuonCore;

MuonCore *mu_simple_core(MuonEngine *engine, MuonName *name)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonCore *muon_record_core(
    MuonEngine *engine, size_t argc, MuonCoreMember argv[const /* argv */])
  MUON_HINT_SUFFIX(nonnull);

/// Emit debugging information on the abstract @a core to the debug stream
void muon_core_debug(MuonCore *core)
  MUON_HINT_SUFFIX(nonnull);

#endif /* MUON_ENGINE_CORE_H */
