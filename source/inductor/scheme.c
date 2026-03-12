#include "scheme.h"

#include "../engine.h"
#include "common.h"
#include "induce.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

/// Like type_next2, but takes an explicit index instead of using cursor->i.
/// Safe to call while the type's header union is being used for series walking.
static inline Attitude type_next2_i(
    const Inductor *inductor, Attitude origin, size_t *ip) {
  if (origin.type->tag != MUON_VARIABLE_TYPE)
    return type_at(origin, (*ip)++);

  for (size_t i; (i = (*ip)++) < inductor->rule_length;) {
    const Rule *edge = &inductor->edge[i];
    if (edge->vertex[!origin.charge] == origin.type)
      return (Attitude) {edge->vertex[origin.charge], origin.charge};
  }

  return (Attitude) {};
}

static inline Attitude type_scan3(const Inductor *inductor, Attitude origin) {
  struct TypeCursor *cursor = type_cursor(origin);
  size_t i = cursor->i;
  Attitude result = type_next2_i(inductor, origin, &i);
  cursor->i = i;
  return result;
}

static struct MuonType *type_allocate(MuonEngine *engine, MuonType *origin)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonType *scheme_instance(MuonInductor *inductor, MuonSchemeType *scheme) {
  Attitude series = {};
  size_t length = 1;

  Attitude cursor = {scheme->matter, 0};
  do {
    Attitude next;
    while (!attitude_null(next = type_scan3(inductor, cursor))) {
      // Skip the type if it's been accessed
      if (type_cursor(next)->type != NULL || type_cursor(next)->i != 0)
        continue;

      // Skip the type unless the scheme is, or is an ancestor of, the type's
      // scheme
      MuonSchemeType *next_scheme = next.type->scheme;
      do {
        if (next_scheme == scheme)
          goto type_continue;
      } while ((next_scheme = next_scheme->as_type.scheme) != NULL);
      continue;

    type_continue:
      cursor = type_continue(cursor, next);
    }

    cursor = type_return(next = cursor);

    type_series(series = type_attach(series, next))->n = length++;
  } while (!attitude_null(cursor));

  struct MuonType *equation[length];
  for (size_t i = 0; i < length; i++)
    equation[i] = NULL;

  // Allocate new types
  cursor = series;
  do {
    cursor = type_next1(cursor);

    struct MuonType *allocation;
    if ((allocation = type_allocate(inductor->engine, cursor.type)) == NULL)
      return NULL;
    equation[type_series(cursor)->n] = allocation;
  } while (!attitude_eq(cursor, series));

#define map_of(type, charge) __extension__ ({ \
  MuonType *_type = (type); \
  size_t _n = type_series((Attitude){_type, charge})->n; \
  assert(_n < length); \
  if (_n < length && equation[_n] != NULL) \
    _type = equation[_n]; \
  _type; \
})

  do {
    cursor = type_next1(cursor);

    MuonType *origin = cursor.type;

    struct MuonType *result = equation[type_series(cursor)->n];
    assert(result != NULL);

    MuonSchemeType *map_scheme = (MuonSchemeType *) map_of(
        &result->scheme->as_type, 0);
    result->scheme = map_scheme;

    switch ON_ABSTRACT_OBJECT(origin) {
      case IS_CONCRETE_TYPE(MuonCoreType *core_type) {
        struct MuonCoreType *allocation = (struct MuonCoreType *) result;

        for (size_t j = 0; j < core_type->core->argc; j++) {
          MuonCoreMember member = core_type->core->argv[j];
          MuonType *argument = core_type->argv[member.i];
          _Bool variance = cursor.charge ^ member.variance;
          allocation->argv[j] = map_of(argument, variance);
        }

        if (core_type_activate(allocation) == NULL)
          return NULL;
        break;
      }

      case IS_CONCRETE_TYPE(MuonSchemeType *scheme_type) {
        struct MuonSchemeType *allocation = (struct MuonSchemeType *) result;

        MuonType *matter = map_of(scheme_type->matter, cursor.charge);

        if (scheme_type_activate(allocation, matter) == NULL)
          return NULL;

        break;
      }

      case IS_CONCRETE_TYPE(MuonJoinType *join_type) {
        struct MuonJoinType *allocation = (struct MuonJoinType *) result;

        for (size_t j = 0; j < join_type->argc; j++)
          allocation->argv[j] = map_of(join_type->argv[j], cursor.charge);

        if (join_type_activate(allocation) == NULL)
          return NULL;
        break;
      }

      case IS_CONCRETE_TYPE(MuonMeetType *meet_type) {
        struct MuonMeetType *allocation = (struct MuonMeetType *) result;

        for (size_t j = 0; j < meet_type->argc; j++)
          allocation->argv[j] = map_of(meet_type->argv[j], cursor.charge);

        if (meet_type_activate(allocation) == NULL)
          return NULL;
        break;
      }

      case MUON_VARIABLE_TYPE: {
        struct MuonVariableType *allocation =
            (struct MuonVariableType *) result;

        if (variable_type_activate(allocation) == NULL)
          return NULL;
        break;
      }
    }
  } while (!attitude_eq(cursor, series));

  size_t rule_length = inductor->rule_length;
  do {
    cursor = type_next1(cursor);

    MuonType *origin = cursor.type;
    if (origin->tag != MUON_VARIABLE_TYPE)
      continue;

    for (size_t j = 0; j < rule_length; j++) {
      Rule *edge = &inductor->edge[j];
      if (edge->instance_id != 0)
        continue;

      if (edge->source != origin && edge->target != origin)
        continue;

      MuonType *new_source = map_of(edge->source, 0);
      MuonType *new_target = map_of(edge->target, 1);

      if (rule_search(inductor, new_source, new_target) != NULL)
        continue;
      if (rule_insert(inductor, new_source, new_target) == NULL)
        return NULL;
    }
  } while (!attitude_eq(cursor, series));

  size_t instance_id = inductor->instance_id++;
  do {
    cursor = type_next1(cursor);

    MuonType *origin = cursor.type;

    struct MuonType *result = equation[type_series(cursor)->n];
    assert(result != NULL);

    if (origin == result)
      continue;

    if (!is_variable_type(origin))
      continue;

    Rule *rule;
    if (cursor.charge == 0) {
      rule = rule_insert(inductor, origin, result);
      rule->instance_id = instance_id;
    } else {
      rule = rule_insert(inductor, result, origin);
      rule->instance_id = instance_id;
    }
  } while (!attitude_eq(cursor, series));

  MuonType *result = map_of(scheme->matter, 0);
  assert(result != NULL);

#undef map_of

  while (!attitude_null(type_detach(series))) {}
  return result;
}

static struct MuonType *type_allocate(MuonEngine *engine, MuonType *origin) {
  switch ON_ABSTRACT_OBJECT(origin) {
    case IS_CONCRETE_TYPE(MuonCoreType *core_type) {
      MuonCore *core = core_type->core;

      struct MuonCoreType *allocation;
      if ((allocation = core_type_allocate(engine, core)) == NULL)
        return NULL;
      return &allocation->as_type;
    }

    case IS_CONCRETE_TYPE(MuonJoinType *join_type) {
      struct MuonJoinType *allocation;
      if ((allocation = join_type_allocate(engine, join_type->argc)) == NULL)
        return NULL;
      return &allocation->as_type;
    }

    case IS_CONCRETE_TYPE(MuonMeetType *meet_type) {
      struct MuonMeetType *allocation;
      if ((allocation = meet_type_allocate(engine, meet_type->argc)) == NULL)
        return NULL;
      return &allocation->as_type;
    }

    case MUON_SCHEME_TYPE: {
      struct MuonSchemeType *allocation;
      if ((allocation = scheme_type_allocate(engine)) == NULL)
        return NULL;
      return &allocation->as_type;
    }

    case MUON_VARIABLE_TYPE: {
      struct MuonVariableType *allocation;
      if ((allocation = variable_type_allocate(engine)) == NULL)
        return NULL;
      return &allocation->as_type;
    }
  }
}
