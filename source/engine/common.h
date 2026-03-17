#ifndef MUON_ENGINE_COMMON_I
#define MUON_ENGINE_COMMON_I

#include <muon/engine.h> // IWYU pragma: export

#include "../common.h" // IWYU pragma: export
#include "../hash.h"   // IWYU pragma: export

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
  size_t node_number[MUON_NODE_NUMBER];
  size_t core_length;
  size_t type_number;
  size_t instance_number;

  MuonName *boolean_name;
  MuonName *integer_name;
  MuonName *lambda_name;

  MuonCore *boolean_core;
  MuonCore *integer_core;
  MuonCore *lambda_core;
  MuonCore *vector_core;

  MuonJoinType *bottom_type;
  MuonMeetType *object_type;

  struct MuonSchemeType *scheme;

  MuonCore *core[200];

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

/// @internal Allocate an object of the @a size in the @a engine
MUON_HINT(malloc, nonnull)
static inline void *engine_allocate2(Engine *engine, size_t size) {
  return malloc(size);
}

#endif /* MUON_ENGINE_COMMON_I */
