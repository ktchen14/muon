#ifndef MUON_ENGINE_COMMON_I
#define MUON_ENGINE_COMMON_I

#include <muon/engine/common.h> // IWYU pragma: export
#include <muon/engine/name.h>

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct {
  union {
    MuonEngine as_engine;

    struct {
      size_t name_number;
      size_t node_number;

      MuonName *name[256];
    };
  };
} Engine;

static_assert(sizeof(MuonEngine) <= sizeof(Engine));

/// Return @a engine as an <tt>Engine *</tt> or <tt>const Engine *</tt>
#define as_engine(engine) ((typeof(_Generic((engine), \
  MuonEngine *: (Engine *) {}, const MuonEngine *: (const Engine *) {} \
))) (engine))

#ifdef MUON_ENGINE_MODULE

/// @internal Allocate an object of the @a size in the @a engine
__attribute__((malloc, nonnull))
static inline void *engine_allocate(MuonEngine *engine, size_t size) {
  return malloc(size);
}

#endif

#endif /* MUON_ENGINE_COMMON_I */
