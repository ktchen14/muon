#ifndef MUON_ENGINE_COMMON_I
#define MUON_ENGINE_COMMON_I

#include <muon/engine.h> // IWYU pragma: export

#include "../common.h" // IWYU pragma: export

#include <stdlib.h>

typedef struct {
  size_t node_number[MUON_NODE_NUMBER];
  size_t type_number;
  size_t instance_number;

  MuonCore *boolean_core;
  MuonCore *integer_core;
  MuonCore *lambda_core;
  MuonCore *vector_core;

  MuonJoinType *bottom_type;
  MuonMeetType *object_type;

  struct MuonSchemeType *scheme;

  HashArea *stator;
} Engine;

/// Return @a engine as an <tt>Engine *</tt> or <tt>const Engine *</tt>
#define as_engine(engine) ((typeof(_Generic((engine), \
  MuonEngine *: (Engine *) {}, const MuonEngine *: (const Engine *) {} \
))) (engine))

/// @internal Allocate an object of the @a size in the @a engine
MUON_HINT(malloc, nonnull)
static inline void *engine_allocate(MuonEngine *engine, size_t size) {
  return malloc(size);
}

#endif /* MUON_ENGINE_COMMON_I */
