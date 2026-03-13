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
  [[maybe_unused]] size_t instance_id = inductor->instance_id++;

  Attitude series = {};

  Attitude cursor = {scheme->matter, 0};
  do {
    Attitude next;
    while (!attitude_null(next = type_scan3(inductor, cursor))) {
      _Bool found = 0;
      for (MuonSchemeType *s = next.type->scheme; s != NULL;
          s = s->as_type.scheme) {
        if (s == scheme) {
          found = 1;
          break;
        }
      }
      if (!found)
        goto next;

      if (type_cursor(next)->type != NULL || type_cursor(next)->i != 0)
        continue;

      cursor = type_continue(cursor, next);
    next:
    }

    cursor = type_return(next = cursor);
    series = type_attach(series, next);
  } while (!attitude_null(cursor));

  // Assign a new offset to each type to duplicate. Calculate the total length
  // we need.
  size_t length = 0;

  cursor = series;
  do {
    cursor = type_next1(cursor);

    size_t n;
    if ((n = type_series(attitude_invert(cursor))->n) == 0)
      n = ++length;
    type_series(cursor)->n = n;
  } while (!attitude_eq(cursor, series));

  ++length;

  struct MuonType *equation[length];
  for (size_t i = 0; i < length; i++)
    equation[i] = NULL;

  // Allocate new types
  do {
    cursor = type_next1(cursor);

    if (equation[type_series(cursor)->n] != NULL)
      continue;

    struct MuonType *result;
    if ((result = type_allocate(inductor->engine, cursor.type)) == NULL)
      return NULL;
    equation[type_series(cursor)->n] = result;
  } while (!attitude_eq(cursor, series));

#define map_of(type) __extension__ ({ \
  MuonType *_type = (type); \
  size_t _n = type_series((Attitude){_type, 0})->n; \
  if (_n == 0) \
    _n = type_series((Attitude){_type, 1})->n; \
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
        &result->scheme->as_type);
    result->scheme = map_scheme;

    switch ON_ABSTRACT_OBJECT(origin) {
      case IS_CONCRETE_TYPE(MuonCoreType *core_type) {
        assert(result->tag == MUON_CORE_TYPE);
        struct MuonCoreType *allocation = (struct MuonCoreType *) result;

        for (size_t j = 0; j < core_type->core->argc; j++)
          allocation->argv[j] = map_of(core_type->argv[j]);

        if (core_type_activate(allocation) == NULL)
          return NULL;
        break;
      }

      case IS_CONCRETE_TYPE(MuonSchemeType *scheme_type) {
        assert(result->tag == MUON_SCHEME_TYPE);
        struct MuonSchemeType *allocation = (struct MuonSchemeType *) result;

        MuonType *matter = map_of(scheme_type->matter);

        if (scheme_type_activate(allocation, matter) == NULL)
          return NULL;

        break;
      }

      case IS_CONCRETE_TYPE(MuonJoinType *join_type) {
        assert(result->tag == MUON_JOIN_TYPE);
        struct MuonJoinType *allocation = (struct MuonJoinType *) result;

        for (size_t j = 0; j < join_type->argc; j++)
          allocation->argv[j] = map_of(join_type->argv[j]);

        if (join_type_activate(allocation) == NULL)
          return NULL;
        break;
      }

      case IS_CONCRETE_TYPE(MuonMeetType *meet_type) {
        assert(result->tag == MUON_MEET_TYPE);
        struct MuonMeetType *allocation = (struct MuonMeetType *) result;

        for (size_t j = 0; j < meet_type->argc; j++)
          allocation->argv[j] = map_of(meet_type->argv[j]);

        if (meet_type_activate(allocation) == NULL)
          return NULL;
        break;
      }

      case MUON_VARIABLE_TYPE: {
        assert(result->tag == MUON_VARIABLE_TYPE);
        struct MuonVariableType *allocation =
            (struct MuonVariableType *) result;

        if (variable_type_activate(allocation) == NULL)
          return NULL;
        break;
      }
    }
  } while (!attitude_eq(cursor, series));

  size_t universe_snapshot = inductor->rule_length;
  do {
    cursor = type_next1(cursor);

    if (cursor.type->tag != MUON_VARIABLE_TYPE)
      continue;

    for (size_t i = 0; i < universe_snapshot; i++) {
      Rule *edge = &inductor->edge[i];

      if (edge->instance_scheme == scheme)
        continue;

      if (edge->vertex[!cursor.charge] != cursor.type)
        continue;

      if (edge->charge[!cursor.charge] != cursor.charge)
        continue;

      MuonType *source = map_of(edge->source);
      MuonType *target = map_of(edge->target);

      if (rule_search(inductor, source, target) != NULL)
        continue;

      Rule *rule;
      if ((rule = rule_insert(inductor, source, target)) == NULL)
        return NULL;
      rule->tag = edge->tag;
      rule->source_charge = edge->source_charge;
      rule->target_charge = edge->target_charge;
      rule->instance_scheme = edge->instance_scheme;
      rule->instance_id = edge->instance_id;
    }
  } while (!attitude_eq(cursor, series));

  do {
    cursor = type_next1(cursor);

    if (!is_variable_type(cursor.type))
      continue;

    MuonType *result = map_of(cursor.type);

    Rule *rule;
    if (cursor.charge == 0)
      rule = rule_insert(inductor, cursor.type, result);
    else
      rule = rule_insert(inductor, result, cursor.type);

    rule->instance_scheme = scheme;
    rule->instance_id = instance_id;
    rule->source_charge = !cursor.charge;
    rule->target_charge = !cursor.charge;
  } while (!attitude_eq(cursor, series));

  MuonType *result = map_of(scheme->matter);
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
