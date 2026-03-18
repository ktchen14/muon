#include "common.h"

#include "../detector/detect.h"
#include "../engine.h"
#include "../inductor.h"
#include "reduce.h"

#include <assert.h>
#include <stdlib.h>

MuonInductor *muon_induce_initialize(
    MuonInductor *inductor,
    MuonEngine *opaque,
    const detect_t *detect,
    const MuonModule *module) {
  assert(detect_result(detect)->engine == opaque);

  MuonType **solution;
  if ((solution = malloc(sizeof(MuonType *[1000]))) == NULL)
    return NULL;
  for (size_t i = 0; i < 1000; i++)
    solution[i] = NULL;

  struct Frontier *frontier;
  if ((frontier = malloc(sizeof(struct Frontier[1000]))) == NULL)
    return NULL;
  for (size_t i = 0; i < 1000; i++)
    frontier[i] = (struct Frontier) {};

  size_t universe_volume = 1000;
  Rule *universe_data;
  if ((universe_data = malloc(sizeof(Rule[universe_volume]))) == NULL)
    return NULL;

  *inductor = (MuonInductor) {
    .engine = opaque,
    .detect = detect_result(detect),
    .module = module,
    .type_length = 1000,
    .solution = solution,
    .frontier = frontier,
    .rule_volume = universe_volume,
    .edge = universe_data,
  };

  Engine *engine = as_engine(opaque);

  size_t offset = 0;
  for (size_t i = 0; i < MUON_NODE_NUMBER; i++) {
    size_t j = i - MUON_MINORANT_NODE;
    inductor->node_offset[j] = offset;
    inductor->node_number[j] = engine->node_number[j];
    offset += engine->node_number[j];
  }

  struct NodeType *node;
  if ((node = malloc(sizeof(struct NodeType[offset]))) == NULL)
    return NULL;
  for (size_t i = 0; i < offset; i++)
    node[i] = (struct NodeType) {};
  inductor->node = node;

  return inductor;
}

void (inductor_debug)( //-
    const MuonInductor *inductor, struct InductorDebugArgs args) {
  debug("digraph muon {\n");
  debug("  rankdir=\"BT\";\n");
  debug("  dpi=192;\n");

  MuonType *roster[inductor->type_length] = {};

  for (size_t i = 0; i < inductor->rule_length; i++) {
    Rule edge = inductor->edge[i];

    Attitude source = attitude_decode(edge.source);
    Attitude target = attitude_decode(edge.target);

    if (args.hide & 1 << edge.tag)
      continue;

    if (roster[source.type->id] == NULL) {
      debug("  Type%zu [label=\"", source.type->id);
      (muon_type_debug)(source.type, args.type);
      debug("\"];\n");
      roster[source.type->id] = source.type;
    }

    if (roster[target.type->id] == NULL) {
      debug("  Type%zu [label=\"", target.type->id);
      (muon_type_debug)(target.type, args.type);
      debug("\"];\n");
      roster[target.type->id] = target.type;
    }

    debug("  Type%zu -> Type%zu [", source.type->id, target.type->id);

    if (source.charge == 0)
      debug("dir=both,arrowtail=odot,");

    if (target.charge == 1)
      debug("arrowhead=odotnormal,");

    switch (edge.tag) {
      case NORMAL_RULE:
      case JOIN_RULE:
        break;

      case INDIRECT_RULE:
        debug("color=gray,");
        break;

      case ID_RULE:
        debug("color=blue,");
        break;

      case IMPOSSIBLE_RULE:
        debug("color=red,constraint=false,");
        break;

      case INSTANCE_RULE:
        assert(edge.instance != NULL);
        debug("color=green,fontcolor=green,label=\"%zu\",", edge.instance->id);
        break;
    }

    debug("];\n");

    if (edge.instance != NULL) {
      debug("  {\n");
      debug("    rank=same;\n");
      debug("    Type%zu [label=\"", source.type->id);
      (muon_type_debug)(source.type, args.type);
      debug("\"];\n");

      debug("    Type%zu [label=\"", target.type->id);
      (muon_type_debug)(target.type, args.type);
      debug("\"];\n");
      debug("  }\n");
    }
  }

  debug("}\n");
}
