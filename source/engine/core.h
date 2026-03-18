#ifndef MUON_ENGINE_CORE_I
#define MUON_ENGINE_CORE_I

#include <muon/engine/core.h>

#include "common.h"
#include "name.h"

#include <assert.h>
#include <stddef.h>

/// @internal Used in ON_ABSTRACT_CORE()
static _Thread_local const void *abstract_core;

/// Used with IS_CONCRETE_CORE() to switch on the tag of the abstract @a core
#define ON_ABSTRACT_CORE(core) ((typeof(core)) {abstract_core = (core)}->tag)

/// Emit a case within a switch ON_ABSTRACT_CORE()
#define IS_CONCRETE_CORE(...) \
  MUON_CORE_TAG(typeof((struct { __VA_ARGS__, *_; }) {}._)): \
    __VA_ARGS__ = abstract_core;

[[gnu::nonnull, gnu::pure]] static inline size_t core_argc(MuonCore *core) {
  switch ON_ABSTRACT_CORE(core) {
    case MUON_BOOLEAN_CORE:
      return 0;

    case IS_CONCRETE_CORE(MuonCustomCore *custom_core)
      return custom_core->argc;

    case MUON_INTEGER_CORE:
      return 0;

    case MUON_LAMBDA_CORE:
      return 2;

    case IS_CONCRETE_CORE(MuonRecordCore *record_core)
      return record_core->argc;

    case MUON_VECTOR_CORE:
      return 1;
  }
}

/// Return the <em>i</em>th member in the abstract @a core
[[gnu::nonnull, gnu::pure]]
static inline MuonCoreMember core_at(MuonCore *core, size_t i) {
  switch ON_ABSTRACT_CORE(core) {
    case MUON_BOOLEAN_CORE:
      unreachable();

    case IS_CONCRETE_CORE(MuonCustomCore *custom_core)
      return custom_core->argv[i];

    case MUON_INTEGER_CORE:
      unreachable();

    case MUON_LAMBDA_CORE:
      return (MuonCoreMember) {.i = i, .variance = !i};

    case IS_CONCRETE_CORE(MuonRecordCore *record_core)
      return (MuonCoreMember) {.name = record_core->argv[i], .i = i};

    case MUON_VECTOR_CORE:
      return (MuonCoreMember) {};
  }
}

struct MuonRecordCore *record_core_allocate(MuonEngine *engine, size_t argc)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonRecordCore *record_core_activate(struct MuonRecordCore *core)
  MUON_HINT_SUFFIX(nonnull);

/// Compare the core member @a a to the core member @a b
MUON_HINT(nonnull, pure)
static inline int core_member_cmp(const void *a, const void *b) {
  const MuonCoreMember *ra = a, *rb = b;
  assert(ra->name != NULL && rb->name != NULL);
  return name_cmp(ra->name, rb->name);
}

#endif /* MUON_ENGINE_CORE_I */
