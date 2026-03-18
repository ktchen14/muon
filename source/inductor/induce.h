#ifndef MU_INDUCTOR_INDUCE_I
#define MU_INDUCTOR_INDUCE_I

#include "common.h"

#include "../engine.h"

#include <assert.h>
#include <stddef.h>

void *induce_script(MuonInductor *inductor, MuonScript *script)
  MUON_HINT_SUFFIX(nonnull);

Rule *type_restrain(
    Inductor *inductor, MuonType *source, MuonType *target, MuonNode *reason)
  MUON_HINT_SUFFIX(nonnull(1, 2, 3));

Rule *type_assess(
    Inductor *inductor, MuonType *restrict source, MuonType *restrict target);

/// Return the type of the @a node in the @a inductor
MUON_HINT(nonnull, pure, returns_nonnull)
static inline MuonType *node_source_type(
    const Inductor *inductor, MuonNode *node) {
  assert(node->engine == inductor->engine);
  assert(node->id < inductor->node_number[node->tag]);

  size_t offset = inductor->node_offset[node->tag];
  MuonType *result = inductor->node[offset + node->id].source;
  return assert(result != NULL), result;
}

/// Return the type of the @a node in the @a inductor
MUON_HINT(nonnull, pure)
static inline MuonType *node_target_type(
    const Inductor *inductor, MuonNode *node) {
  assert(node->engine == inductor->engine);
  assert(node->id < inductor->node_number[node->tag]);

  size_t offset = inductor->node_offset[node->tag];
  return inductor->node[offset + node->id].target;
}

#endif /* MU_INDUCTOR_INDUCE_I */
