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
} RuleTag;

typedef struct {
  union {
    MUON_HINT(packed) struct {
      MuonType *source;
      MuonType *target;
    };

    MuonType *vertex[2];
  };

  _Bool hidden[2];

  RuleTag tag;

  MuonInstance *instance; // optional

  MuonNode *reason; // optional

  union {
    MuonType *center;
    size_t i;
  };
} Rule;

typedef struct {
  MuonType *type; // variant for instance == NULL
  size_t argc;
  struct SolutionItem {
    MuonInstance *instance;
    MuonType *type;
  } argv[] MUON_HINT(counted_by(argc));
} Solution;

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
  Solution **attitude_solution; // indexed by type_id * 2 + charge

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
    if (edge->source == source && edge->target == target)
      return edge;
  }
  return NULL;
}

[[gnu::nonnull]] static inline RuleIterator rule_iterator(
    const Inductor *inductor, Attitude attitude) {
  return (RuleIterator) {.inductor = inductor, .attitude = attitude};
}

[[gnu::nonnull]] static inline Rule *rule_next(RuleIterator *it) {
  _Bool charge = it->attitude.charge;
  for (size_t i; (i = it->i++) < it->inductor->rule_length;) {
    Rule *edge = &it->inductor->edge[i];
    if (edge->hidden[charge])
      continue;
    if (edge->vertex[!charge] != it->attitude.type)
      continue;
    return edge;
  }

  return NULL;
}

/// Find rules on the opposite side from rule_next.
///
/// rule_scan(v, !c) finds every edge v → t that rule_next(t, c) would find.
///
/// rule_scan({v, c}) finds rules where:
///   - vertex[!c] == v  (v is on the !c side)
///   - hidden[!c] is false  (not hidden from rule_next(t, !c) on the other
///   side)
[[gnu::nonnull]] static inline Rule *rule_scan(RuleIterator *it) {
  _Bool charge = it->attitude.charge;
  for (size_t i; (i = it->i++) < it->inductor->rule_length;) {
    Rule *edge = &it->inductor->edge[i];
    if (edge->hidden[!charge])
      continue;
    if (edge->vertex[!charge] != it->attitude.type)
      continue;
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
    .source = source,
    .target = target,
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

[[gnu::nonnull, gnu::pure]]
static inline Solution *attitude_solution_get(
    const Inductor *inductor, Attitude attitude) {
  assert(attitude.type->id < inductor->type_length);
  return inductor->attitude_solution[attitude.type->id * 2 + attitude.charge];
}

[[gnu::nonnull]]
static inline void attitude_solution_set(
    Inductor *inductor, Attitude attitude, Solution *solution) {
  assert(attitude.type->id < inductor->type_length);
  inductor->attitude_solution[attitude.type->id * 2 + attitude.charge] =
      solution;
}

static inline Attitude type_next(const Inductor *inductor, Attitude origin) {
  if (origin.type->tag != MUON_VARIABLE_TYPE)
    return type_at(origin, type_cursor(origin)->i++);

  size_t i;
  while ((i = type_cursor(origin)->i++) < inductor->rule_length) {
    const Rule *rule = &inductor->edge[i];

    if (rule->hidden[origin.charge])
      continue;

    if (rule->vertex[!origin.charge] != origin.type)
      continue;

    return (Attitude) {rule->vertex[origin.charge], origin.charge};
  }

  return (Attitude) {};
}

#endif /* MUON_INDUCTOR_COMMON_I */
