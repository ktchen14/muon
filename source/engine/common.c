#include "common.h"
#include "stator.h"

#include "../hash.h"

#include <assert.h>
#include <errno.h>
#include <stdckdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

MuonEngine *muon_engine_initialize(MuonEngine *opaque) {
  Engine *engine = as_engine(opaque);

  size_t area_size = 16;
  HashArea *hash_area;
  struct_size_overflow(HashArea, item, &area_size);
  if ((hash_area = malloc(area_size)) == NULL)
    return NULL;
  *hash_area = (HashArea) {.volume = 16};
  for (size_t i = 0; i < 16; i++)
    hash_area->item[i] = (struct HashItem) {};

  *engine = (Engine) {.stator = hash_area};

  struct MuonBooleanCore *boolean_core;
  if ((boolean_core = engine_allocate(opaque, sizeof(MuonBooleanCore))) == NULL)
    return NULL;
  *boolean_core = (MuonBooleanCore) {
    .as_core = {.tag = MUON_BOOLEAN_CORE, .engine = opaque}
  };
  engine->boolean_core = &boolean_core->as_core;

  struct MuonIntegerCore *integer_core;
  if ((integer_core = engine_allocate(opaque, sizeof(MuonIntegerCore))) == NULL)
    return NULL;
  *integer_core = (MuonIntegerCore) {
    .as_core = {.tag = MUON_INTEGER_CORE, .engine = opaque}
  };
  engine->integer_core = &integer_core->as_core;

  struct MuonLambdaCore *lambda_core;
  if ((lambda_core = engine_allocate(opaque, sizeof(MuonLambdaCore))) == NULL)
    return NULL;
  *lambda_core = (MuonLambdaCore) {
    .as_core = {.tag = MUON_LAMBDA_CORE, .engine = opaque}
  };
  engine->lambda_core = &lambda_core->as_core;

  struct MuonVectorCore *vector_core;
  if ((vector_core = engine_allocate(opaque, sizeof(MuonVectorCore))) == NULL)
    return NULL;
  *vector_core = (MuonVectorCore) {
    .as_core = {.tag = MUON_VECTOR_CORE, .engine = opaque}
  };
  engine->vector_core = &vector_core->as_core;

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
