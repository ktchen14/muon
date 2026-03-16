#include "common.h"
#include "stator.h"

#include <assert.h>
#include <errno.h>
#include <stdckdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

MuonEngine *muon_engine_initialize(MuonEngine *opaque) {
  Engine *engine = as_engine(opaque);

  size_t stator_volume = 16;
  struct HashStator *stator;
  if ((stator = malloc(sizeof(struct HashStator [stator_volume]))) == NULL)
    return NULL;

  for (size_t i = 0; i < stator_volume; i++)
    stator[i] = (struct HashStator) {};

  *engine = (Engine) {.stator_volume = stator_volume, .stator = stator};

  MuonName *boolean_name;
  if ((boolean_name = muon_name(opaque, strlen("Boolean"), "Boolean")) == NULL)
    return NULL;
  engine->boolean_name = boolean_name;

  struct MuonCore *boolean_core;
  if ((boolean_core = engine_allocate(opaque, sizeof(MuonCore))) == NULL)
    return NULL;
  *boolean_core = (MuonCore) {
    .tag = MUON_BOOLEAN_CORE, .engine = opaque, .name = boolean_name
  };
  engine->boolean_core = boolean_core;

  MuonName *integer_name;
  if ((integer_name = muon_name(opaque, strlen("Integer"), "Integer")) == NULL)
    return NULL;
  engine->integer_name = integer_name;

  struct MuonCore *integer_core;
  if ((integer_core = engine_allocate(opaque, sizeof(MuonCore))) == NULL)
    return NULL;
  *integer_core = (MuonCore) {
    .tag = MUON_INTEGER_CORE, .engine = opaque, .name = integer_name
  };
  engine->integer_core = integer_core;

  MuonName *lambda_name;
  if ((lambda_name = muon_name(opaque, strlen("λ"), "λ")) == NULL)
    return NULL;
  engine->lambda_name = lambda_name;

  size_t size = struct_size(MuonCore, argv, 2);
  struct MuonCore *lambda_core;
  if ((lambda_core = engine_allocate(opaque, size)) == NULL)
    return NULL;
  *lambda_core = (MuonCore) {
    .tag = MUON_LAMBDA_CORE, .engine = opaque, .name = lambda_name, .argc = 2
  };
  lambda_core->argv[0] = (MuonCoreMember) {.i = 0, .variance = 1};
  lambda_core->argv[1] = (MuonCoreMember) {.i = 1};
  engine->lambda_core = lambda_core;

  size = struct_size(MuonCore, argv, 1);
  struct MuonCore *vector_core;
  if ((vector_core = engine_allocate(opaque, size)) == NULL)
    return NULL;
  *vector_core = (MuonCore) {
    .tag = MUON_VECTOR_CORE, .engine = opaque, .argc = 1
  };
  vector_core->argv[0] = (MuonCoreMember) {};
  engine->vector_core = vector_core;

  MuonJoinType *bottom_type;
  if ((bottom_type = muon_join_type(opaque, 0, NULL)) == NULL)
    return NULL;
  engine->bottom_type = bottom_type;

  MuonMeetType *object_type;
  if ((object_type = muon_meet_type(opaque, 0, NULL)) == NULL)
    return NULL;
  engine->object_type = object_type;

  return opaque;
}

Engine *stator_rehash(Engine *engine, MuonStator *stator, Hash hash, size_t *i) {
  size_t size;
  if (ckd_mul(&size, engine->stator_volume, sizeof(struct HashStator) * 2))
    return errno = ENOMEM, NULL;
  size_t volume = engine->stator_volume * 2;

  struct HashStator *area;
  if ((area = malloc(size)) == NULL)
    return NULL;
  for (size_t i = 0; i < volume; i++)
    area[i] = (struct HashStator) {};

  volume = MOVE(engine->stator_volume, volume);
  area = MOVE(engine->stator, area);

  for (size_t j = 0, i; j < volume; j++) {
    struct HashStator next;
    if ((next = area[j]).stator == NULL)
      continue;

    i = stator_slot(engine, next.hash, next.hash);
    while ((next = MOVE(engine->stator[i], next)).stator != NULL)
      i = stator_slot(engine, next.hash, i + 1);
  }

  *i = stator_slot(engine, hash, hash);
  return engine;
}
