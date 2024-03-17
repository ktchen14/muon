#ifndef MUON_ENGINE_CORE_I
#define MUON_ENGINE_CORE_I

#include <muon/engine/core.h>

#include "common.h"
#include "name.h"

#include <assert.h>
#include <stddef.h>

struct MuonCore *record_core_allocate(MuonEngine *engine, size_t argc)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonCore *record_core_activate(struct MuonCore *core)
  MUON_HINT_SUFFIX(nonnull);

/// Compare the core member @a a to the core member @a b
MUON_HINT(nonnull, pure)
static inline int core_member_cmp(const void *a, const void *b) {
  const MuonCoreMember *ra = a, *rb = b;
  assert(ra->name != NULL && rb->name != NULL);
  return name_cmp(ra->name, rb->name);
}

#endif /* MUON_ENGINE_CORE_I */
