#include "instance.h"

#include "common.h"

#include <assert.h>

/// @internal Return the mutable engine of the @a instance
static inline MuonEngine *unlock_engine(struct MuonInstance *instance) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
  return (MuonEngine *) instance->engine;
#pragma GCC diagnostic pop
}

MuonInstance *muon_instance(MuonEngine *engine, MuonSchemeType *scheme) {
  struct MuonInstance *result;
  if ((result = instance_allocate(engine)) == NULL)
    return NULL;
  return instance_activate(result, scheme);
}

struct MuonInstance *instance_allocate(MuonEngine *engine) {
  struct MuonInstance *result;
  if ((result = engine_allocate(engine, sizeof(MuonInstance))) == NULL)
    return NULL;
  *result = (MuonInstance) {.engine = engine};
  return result;
}

MuonInstance *instance_activate(
    struct MuonInstance *instance, MuonSchemeType *scheme) {
  MuonEngine *engine = unlock_engine(instance);

  assert(scheme->as_stator.engine == engine);

  instance->id = as_engine(engine)->instance_number++;
  instance->scheme = scheme;
  return instance;
}
