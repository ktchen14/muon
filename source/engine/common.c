#include "common.h"

#include <stdlib.h>

MuonEngine *muon_engine_initialize(MuonEngine *engine) {
  struct MuonBooleanCore *boolean_core;
  if ((boolean_core = engine_allocate(engine, sizeof(MuonBooleanCore))) == NULL)
    return NULL;
  *boolean_core = (MuonBooleanCore) {
    .as_core = {.tag = MUON_BOOLEAN_CORE, .engine = engine}
  };

  struct MuonIntegerCore *integer_core;
  if ((integer_core = engine_allocate(engine, sizeof(MuonIntegerCore))) == NULL)
    return NULL;
  *integer_core = (MuonIntegerCore) {
    .as_core = {.tag = MUON_INTEGER_CORE, .engine = engine}
  };

  struct MuonLambdaCore *lambda_core;
  if ((lambda_core = engine_allocate(engine, sizeof(MuonLambdaCore))) == NULL)
    return NULL;
  *lambda_core = (MuonLambdaCore) {
    .as_core = {.tag = MUON_LAMBDA_CORE, .engine = engine}
  };

  struct MuonVectorCore *vector_core;
  if ((vector_core = engine_allocate(engine, sizeof(MuonVectorCore))) == NULL)
    return NULL;
  *vector_core = (MuonVectorCore) {
    .as_core = {.tag = MUON_VECTOR_CORE, .engine = engine}
  };

  HashVector *stator;
  if ((stator = hash_vector(16)) == NULL)
    return NULL;

  *as_engine(engine) = (Engine) {
    .boolean_core = &boolean_core->as_core,
    .integer_core = &integer_core->as_core,
    .lambda_core = &lambda_core->as_core,
    .vector_core = &vector_core->as_core,
    .stator = stator
  };

  MuonJoinType *bottom_type;
  if ((bottom_type = muon_join_type(engine, 0, NULL)) == NULL)
    return NULL;
  as_engine(engine)->bottom_type = bottom_type;

  MuonMeetType *object_type;
  if ((object_type = muon_meet_type(engine, 0, NULL)) == NULL)
    return NULL;
  as_engine(engine)->object_type = object_type;

  return engine;
}
