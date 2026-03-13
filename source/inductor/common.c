#include "common.h"

#include "../detector/detect.h"
#include "../engine.h"
#include "reduce.h"

#include <assert.h>
#include <stdlib.h>

MuonInductor *muon_induce_initialize(
    MuonInductor *inductor,
    MuonEngine *engine,
    const detect_t *detect,
    const MuonModule *module) {
  assert(detect_result(detect)->engine == engine);

  size_t node_length = as_engine(engine)->node_number;

  struct NodeType *node;
  if ((node = malloc(sizeof(struct NodeType[node_length]))) == NULL)
    return NULL;
  for (size_t i = 0; i < node_length; i++)
    node[i] = (struct NodeType) {};

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
    .engine = engine,
    .detect = detect_result(detect),
    .module = module,
    .node_length = node_length,
    .node = node,
    .type_length = 1000,
    .solution = solution,
    .frontier = frontier,
    .rule_volume = universe_volume,
    .edge = universe_data,
    .instance_id = 1,
  };

  return inductor;
}

void (inductor_debug)( //-
    const MuonInductor *inductor, struct MuonTypeDebugArgs args) {
  debug("digraph muon {\n");
  debug("  rankdir=\"BT\";\n");
  debug("  dpi=192;\n");

  for (size_t i = 0; i < inductor->rule_length; i++) {
    Rule edge = inductor->edge[i];
    Attitude source = attitude_decode(edge.source);
    Attitude target = attitude_decode(edge.target);

    debug("  Type%zu [label=\"", source.type->id);
    (muon_type_debug)(source.type, args);
    debug("\"];\n");

    debug("  Type%zu [label=\"", target.type->id);
    (muon_type_debug)(target.type, args);
    debug("\"];\n");

    _Bool show_indirect = 0;
    if (edge.tag == INDIRECT_RULE) {
      if (!show_indirect)
        continue;

      // MuonCoreType *c;
      // if ((c = muon_type_cast(target.type, c)) == NULL)
      //   continue;
      // if (c->core != as_engine(c->as_stator.engine)->boolean_core)
      //   continue;

      debug(
          "  Type%zu -> Type%zu [constraint=false,color=gray];\n",
          source.type->id,
          target.type->id);
      continue;
    }

    debug("  Type%zu -> Type%zu", source.type->id, target.type->id);

    // With rankdir=BT, Graphviz flips the digraph so s refers to the top of a
    // node while n refers to the bottom of a node.
    const char *attr;
    if (source.charge == 1 && target.charge == 0)
      attr = "tailport=s,headport=n";
    else if (source.charge == 0 && target.charge == 1)
      attr = "tailport=n,headport=s";
    else if (source.charge == 1 && target.charge == 1)
      attr = "tailport=s,headport=s";
    else if (source.charge == 0 && target.charge == 0)
      attr = "tailport=n,headport=n";

    if (edge.instance_id != 0) {
      debug(" [%s,label=\"%zu\",color=green];\n", attr, edge.instance_id);

      debug("  {\n");
      debug("    rank=same;\n");
      debug("    Type%zu [label=\"", source.type->id);
      (muon_type_debug)(source.type, args);
      debug("\"];\n");

      debug("    Type%zu [label=\"", target.type->id);
      (muon_type_debug)(target.type, args);
      debug("\"];\n");
      debug("  }\n");
      continue;
    } else if (edge.tag == ID_RULE) {
      debug(" [%s,color=blue]", attr);
    } else if (edge.tag == IMPOSSIBLE_RULE) {
      debug(" [%s,color=red,constraint=false]", attr);
    }

    debug(";\n");
  }

  debug("}\n");
}
