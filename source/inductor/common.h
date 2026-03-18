#ifndef MUON_INDUCTOR_COMMON_I
#define MUON_INDUCTOR_COMMON_I

#include <muon/inductor.h> // IWYU pragma: export

#include "../common.h" // IWYU pragma: export
#include "../detector/detect.h"
#include "../engine.h" // IWYU pragma: export

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>

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
      AttitudeCode source;
      AttitudeCode target;
    };

    AttitudeCode vertex[2];
  };

  RuleTag tag;

  MuonInstance *instance; // optional

  MuonNode *reason; // optional

  union {
    MuonType *center;
    size_t i;
  };
} Rule;

typedef struct {
  MuonType *base; // solution for instance == NULL
  size_t argc;
  struct AttitudeSolutionItem {
    MuonInstance *instance;
    MuonType *type;
  } argv[];
} AttitudeSolution;

struct Inductor {
  MuonEngine *engine;
  const detect_result_t *detect;
  const MuonModule *module;

  size_t node_offset[MUON_NODE_NUMBER + 1];
  struct NodeType {
    MuonType *source;
    MuonType *target;
  } *node;

  size_t type_length;
  MuonType **solution;
  AttitudeSolution **attitude_solution; // indexed by type_id * 2 + charge

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

static inline Attitude type_next(const Inductor *inductor, Attitude origin) {
  if (origin.type->tag != MUON_VARIABLE_TYPE)
    return type_at(origin, type_cursor(origin)->i++);

  size_t i;
  while ((i = type_cursor(origin)->i++) < inductor->rule_length) {
    const Rule *rule = &inductor->edge[i];

    Attitude vertex = attitude_decode(rule->vertex[!origin.charge]);
    if (!attitude_eq(vertex, origin))
      continue;

    MuonType *next = attitude_decode(rule->vertex[origin.charge]).type;
    return (Attitude) {next, origin.charge};
  }

  return (Attitude) {};
}

#endif /* MUON_INDUCTOR_COMMON_I */
