#ifndef MU_INDUCTOR_NODE_I
#define MU_INDUCTOR_NODE_I

#include "common.h"

#include "../engine.h"

#include <assert.h>
#include <stddef.h>

/// Return the source type of the @a node in the @a inductor
[[gnu::nonnull, gnu::pure]] static inline MuonType *node_source_type(
    const Inductor *inductor, MuonNode *node) {
  assert(node->engine == inductor->engine);

  size_t i = node->tag - MUON_MINORANT_NODE;
  const size_t *offset = &inductor->node_offset[i];
  assert(*offset + node->id < offset[1]);

  return inductor->node[*offset + node->id].source;
}

/// Return the target type of the @a node in the @a inductor
[[gnu::nonnull, gnu::pure]] static inline MuonType *node_target_type(
    const Inductor *inductor, MuonNode *node) {
  assert(node->engine == inductor->engine);

  size_t i = node->tag - MUON_MINORANT_NODE;
  const size_t *offset = &inductor->node_offset[i];
  assert(*offset + node->id < offset[1]);

  return inductor->node[*offset + node->id].target;
}

void *induce_script(MuonInductor *inductor, MuonScript *script)
  MUON_HINT_SUFFIX(nonnull);

#endif /* MU_INDUCTOR_NODE_I */

