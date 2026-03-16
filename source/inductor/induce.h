#ifndef MU_INDUCTOR_INDUCE_I
#define MU_INDUCTOR_INDUCE_I

#include "../engine.h"
#include "common.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>

[[gnu::nonnull]] static inline Rule *rule_search(
    const Inductor *inductor, MuonType *source, MuonType *target) {
  for (size_t i = 0; i < inductor->rule_length; i++) {
    Rule *edge = &inductor->edge[i];
    if (attitude_decode(edge->source).type == source
        && attitude_decode(edge->target).type == target)
      return edge;
  }
  return NULL;
}

[[gnu::nonnull]] static inline RuleIterator rule_iterator(
    const Inductor *inductor, Attitude attitude) {
  return (RuleIterator) {.inductor = inductor, .attitude = attitude};
}

[[gnu::nonnull]] static inline Rule *rule_next(RuleIterator *it) {
  for (size_t i; (i = it->i++) < it->inductor->rule_length;) {
    Rule *edge = &it->inductor->edge[i];
    Attitude vertex = attitude_decode(edge->vertex[!it->attitude.charge]);
    if (attitude_eq(vertex, it->attitude))
      return edge;
  }

  return NULL;
}

/**
 * @brief Return the type of the @a node
 *
 * This will traverse each node reachable from the @a node and will add all
 * constraints to the @a inductor.
 */
MuonType *induce_node(MuonInductor *inductor, MuonNode *node)
  MUON_HINT_SUFFIX(nonnull);

void *induce_script(MuonInductor *inductor, MuonScript *script)
  MUON_HINT_SUFFIX(nonnull);

Rule *type_restrain(
    Inductor *inductor, MuonType *source, MuonType *target, MuonNode *reason)
  MUON_HINT_SUFFIX(nonnull(1, 2, 3));

Rule *retrieve_coercion(
    Inductor *inductor, MuonType *restrict source, MuonType *restrict target);

static Rule *rule_insert(
    Inductor *inductor, MuonType *source, MuonType *target) {
  if (inductor->rule_length >= inductor->rule_volume) {
    size_t volume = inductor->rule_volume;
    if (rare(__builtin_mul_overflow(volume, 2, &volume)))
      return errno = ENOMEM, NULL;

    size_t size;
    if (rare(__builtin_mul_overflow(volume, sizeof(Rule), &size)))
      return errno = ENOMEM, NULL;

    Rule *data;
    if ((data = realloc(inductor->edge, size)) == NULL)
      return NULL;
    inductor->edge = data;

    inductor->rule_volume = volume;
  }

  Rule *result = &inductor->edge[inductor->rule_length++];
  *result = (Rule) {
    .source = attitude_encode((Attitude) {source, 1}),
    .target = attitude_encode((Attitude) {target, 0}),
  };
  return result;
}

[[gnu::nonnull]] static inline Rule *edge_define(
    Inductor *inductor, MuonType *source, MuonType *target) {
  Rule *result;
  if ((result = rule_search(inductor, source, target)) != NULL)
    return result;
  return rule_insert(inductor, source, target);
}

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
