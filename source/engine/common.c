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
