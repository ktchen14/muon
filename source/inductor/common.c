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

  Solution **attitude_solution;
  if ((attitude_solution = malloc(sizeof(Solution *[2000]))) == NULL)
    return NULL;
  for (size_t i = 0; i < 2000; i++)
    attitude_solution[i] = NULL;

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
    .attitude_solution = attitude_solution,
    .rule_volume = universe_volume,
    .edge = universe_data,
  };

  Engine *engine = as_engine(opaque);

  size_t offset = 0;
  for (size_t i = 0; i < MUON_NODE_NUMBER; i++) {
    inductor->node_offset[i] = offset;
    offset += engine->node_number[i];
  }
  inductor->node_offset[MUON_NODE_NUMBER] = offset;

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

    MuonType *source_type = edge.source;
    MuonType *target_type = edge.target;

    if (args.hide & 1 << edge.tag)
      continue;

    if (roster[source_type->id] == NULL) {
      debug("  Type%zu [label=\"", source_type->id);

      (muon_type_debug)(source_type, args.type);
      Solution *solution;
      if ((solution = attitude_solution_get(
               inductor, (Attitude) {source_type, 0}))
          != NULL) {
        debug(" [");
        (muon_type_debug)(solution->type, args.type);
        debug("]");
      }

      debug("\"];\n");
      roster[source_type->id] = source_type;
    }

    if (roster[target_type->id] == NULL) {
      debug("  Type%zu [label=\"", target_type->id);

      (muon_type_debug)(target_type, args.type);
      Solution *solution;
      if ((solution = attitude_solution_get(
               inductor, (Attitude) {target_type, 0}))
          != NULL) {
        debug(" [");
        (muon_type_debug)(solution->type, args.type);
        debug("]");
      }

      debug("\"];\n");
      roster[target_type->id] = target_type;
    }

    debug("  Type%zu -> Type%zu [", source_type->id, target_type->id);

    if (edge.locked[1])
      debug("dir=both,arrowtail=odot,");

    if (edge.locked[0])
      debug("arrowhead=odotnormal,");

    if (edge.instance != NULL) {
      debug("color=green,fontcolor=green,label=\"%zu\",", edge.instance->id);
    } else {
      switch (edge.tag) {
        case NORMAL_RULE:
          break;

        case JOIN_RULE:
          debug("color=purple,");
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
      }
    }

    debug("];\n");
  }

  debug("}\n");
}
