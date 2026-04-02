#include "instance.h"

#include "common.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>

/// @internal Return the mutable engine of the @a instance
static inline MuonEngine *unlock_engine(struct MuonInstance *instance) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
  return (MuonEngine *) instance->engine;
#pragma GCC diagnostic pop
}

MuonInstance *muon_instance(
    MuonEngine *engine,
    MuonSchemeType *scheme,
    MuonImplicitType *const argv[]) {
  struct MuonInstance *result;
  if ((result = instance_allocate(engine, scheme)) == NULL)
    return NULL;
  for (size_t i = 0; i < scheme->argc; i++)
    result->argv[i] = argv[i];
  return instance_activate(result);
}

struct MuonInstance *instance_allocate(
    MuonEngine *engine, MuonSchemeType *scheme) {
  assert(scheme->as_type.engine == engine);

  size_t size = scheme->argc;
  if (struct_size_overflow(MuonInstance, argv, &size))
    return errno = ENOMEM, NULL;

  struct MuonInstance *result;
  if ((result = engine_allocate(engine, size)) == NULL)
    return NULL;
  *result = (MuonInstance) {.engine = engine, .scheme = scheme};
  return result;
}

MuonInstance *instance_activate(struct MuonInstance *instance) {
  MuonEngine *engine = unlock_engine(instance);

  instance->id = as_engine(engine)->instance_number++;
  return instance;
}
