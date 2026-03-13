#ifndef MUON_INDUCTOR_COMMON_I
#define MUON_INDUCTOR_COMMON_I

#include <muon/inductor.h> // IWYU pragma: export

#include "../common.h" // IWYU pragma: export
#include "../detector/detect.h"
#include "../engine.h"

#include <stddef.h>

typedef struct Inductor MuonInductor;
typedef struct Inductor Inductor;

typedef enum {
  NORMAL_RULE,
  INDIRECT_RULE,
  ID_RULE,
  JOIN_RULE,
  IMPOSSIBLE_RULE,
  INSTANCE_RULE,
} RuleTag;

typedef struct {
  union {
    MUON_HINT(packed) struct {
      MuonType *source;
      MuonType *target;
    };

    MuonType *vertex[2];
  };

  union {
    MUON_HINT(packed) struct {
      _Bool source_charge;
      _Bool target_charge;
    };
    _Bool *charge[2];
  };

  RuleTag tag;

  MuonSchemeType *instance_scheme;
  size_t instance_id;

  MuonNode *reason; // optional

  union {
    MuonType *center;
    size_t i;
  };
} Rule;

struct Inductor {
  MuonEngine *engine;
  const detect_result_t *detect;
  const MuonModule *module;

  size_t node_length;
  struct NodeType {
    MuonType *source;
    MuonType *target;
  } *node;

  size_t type_length;
  MuonType **solution;
  struct Frontier {
    MuonType *data[2];
  } *frontier;

  /// Incremented by 1 on each instantiation of a MuonSchemeType
  size_t instance_id;

  size_t rule_length;
  size_t rule_volume;
  Rule *edge;

  MuonCore *datatype_core;
};

typedef struct {
  const Inductor *inductor;
  Attitude attitude;
  size_t i;
} RuleIterator;

#endif /* MUON_INDUCTOR_COMMON_I */
