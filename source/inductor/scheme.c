#include "scheme.h"

#include "../engine.h"
#include "common.h"
#include "induce.h"

#include <assert.h>
#include <errno.h>
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
    Attitude source = attitude_decode(edge->source);
    Attitude target = attitude_decode(edge->target);

    if (source.type == origin.type && source.charge == origin.charge)
      return target;

    if (target.type == origin.type && target.charge == origin.charge)
      return source;
  }

  return (Attitude) {};
}

static inline Attitude type_scan3(const Inductor *inductor, Attitude origin) {
  return type_next2_i(inductor, origin, &type_cursor(origin)->i);
}

MuonType *scheme_instance(MuonInductor *inductor, MuonSchemeType *scheme) {
  MuonEngine *engine = inductor->engine;
  size_t instance_id = inductor->instance_id++;

  union {
    struct MuonType *allocation;
    MuonType *result;
  } equation[inductor->type_length] = {};

  Attitude series = {};
  series = type_attach(series, (Attitude) {&scheme->as_type, 0});

  Attitude cursor = {scheme->matter, 0};
  goto entrance;
  do {
    Attitude next;
    while (!attitude_isnull(next = type_scan3(inductor, cursor))) {
      struct TypeCursor *next_cursor = type_cursor(next);
      if (!attitude_isnull(attitude_decode(next_cursor->attitude)))
        continue;
      if (next_cursor->i != 0)
        continue;

      if (next.type->scheme != scheme
          && equation[next.type->scheme->as_type.id].allocation == NULL)
        continue;

      cursor = type_continue(cursor, next);

      if (equation[cursor.type->id].allocation != NULL)
        continue;

    entrance:
      switch ON_ABSTRACT_OBJECT(cursor.type) {
        case IS_CONCRETE_TYPE(MuonCoreType *core_type) {
          MuonCore *core = core_type->core;

          struct MuonCoreType *allocation;
          if ((allocation = core_type_allocate(engine, core)) == NULL)
            goto except;
          equation[cursor.type->id].allocation = &allocation->as_type;
          break;
        }

        case IS_CONCRETE_TYPE(MuonJoinType *join_type) {
          struct MuonJoinType *allocation;
          size_t argc = join_type->argc;
          if ((allocation = join_type_allocate(engine, argc)) == NULL)
            goto except;
          equation[cursor.type->id].allocation = &allocation->as_type;
          break;
        }

        case IS_CONCRETE_TYPE(MuonMeetType *meet_type) {
          struct MuonMeetType *allocation;
          size_t argc = meet_type->argc;
          if ((allocation = meet_type_allocate(engine, argc)) == NULL)
            goto except;
          equation[cursor.type->id].allocation = &allocation->as_type;
          break;
        }

        case MUON_SCHEME_TYPE: {
          struct MuonSchemeType *allocation;
          if ((allocation = scheme_type_allocate(engine)) == NULL)
            goto except;
          equation[cursor.type->id].allocation = &allocation->as_type;
          break;
        }

        case MUON_VARIABLE_TYPE: {
          MuonVariableType *result;
          if ((result = muon_variable_type(engine)) == NULL)
            goto except;
          equation[cursor.type->id].result = &result->as_type;
          break;
        }
      }
    }

    switch ON_ABSTRACT_OBJECT(cursor.type) {
      case IS_CONCRETE_TYPE(MuonCoreType *core_type) {
        struct MuonCoreType *allocation =
            (struct MuonCoreType *) equation[cursor.type->id].allocation;

        MuonCore *core = core_type->core;

        for (size_t i = 0; i < core->argc; i++) {
          MuonCoreMember member = core->argv[i];
          MuonType *argument = core_type->argv[member.i];
          if (equation[argument->id].result != NULL)
            argument = equation[argument->id].result;
          allocation->argv[i] = argument;
        }

        MuonCoreType *result;
        if ((result = core_type_activate(allocation)) == NULL)
          goto except;
        equation[cursor.type->id].result = &result->as_type;
        break;
      }

      case IS_CONCRETE_TYPE(MuonJoinType *join_type) {
        struct MuonJoinType *allocation =
            (struct MuonJoinType *) equation[cursor.type->id].allocation;

        for (size_t i = 0; i < join_type->argc; i++) {
          MuonType *argument = join_type->argv[i];
          if (equation[argument->id].result != NULL)
            argument = equation[argument->id].result;
          allocation->argv[i] = argument;
        }

        MuonJoinType *result;
        if ((result = join_type_activate(allocation)) == NULL)
          goto except;
        equation[cursor.type->id].result = &result->as_type;
        break;
      }

      case IS_CONCRETE_TYPE(MuonMeetType *meet_type) {
        struct MuonMeetType *allocation =
            (struct MuonMeetType *) equation[cursor.type->id].allocation;

        for (size_t i = 0; i < meet_type->argc; i++) {
          MuonType *argument = join_type->argv[i];
          if (equation[argument->id].result != NULL)
            argument = equation[argument->id].result;
          allocation->argv[i] = argument;
        }

        MuonMeetType *result;
        if ((result = meet_type_activate(allocation)) == NULL)
          goto except;
        equation[cursor.type->id].result = &result->as_type;
        break;
      }

      case IS_CONCRETE_TYPE(MuonSchemeType *scheme_type) {
        struct MuonSchemeType *allocation =
            (struct MuonSchemeType *) equation[cursor.type->id].allocation;

        MuonType *matter = scheme_type->matter;
        if (equation[matter->id].result != NULL)
          matter = equation[matter->id].result;

        MuonSchemeType *result;
        if ((result = scheme_type_activate(allocation, matter)) == NULL)
          goto except;
        equation[cursor.type->id].result = &result->as_type;
        break;
      }

      case MUON_VARIABLE_TYPE:
        RuleIterator it = rule_iterator(inductor, cursor);
        for (const Rule *rule; (rule = rule_next(&it)) != NULL;) {
          if (rule->instance_scheme == scheme)
            continue;

          Attitude source = attitude_decode(rule->source);
          if (equation[source.type->id].result != NULL)
            source.type = equation[source.type->id].result;

          Attitude target = attitude_decode(rule->target);
          if (equation[target.type->id].result != NULL)
            target.type = equation[target.type->id].result;

          if (rule_search(inductor, source.type, target.type) != NULL)
            continue;

          Rule *next;
          if ((next = rule_insert(inductor, source.type, target.type)) == NULL)
            goto except;
          next->tag = rule->tag;
          next->source = attitude_encode(source);
          next->target = attitude_encode(target);
          next->instance_scheme = rule->instance_scheme;
          next->instance_id = rule->instance_id;
        }

        MuonType *result = equation[cursor.type->id].result;

        Attitude source = {
          (MuonType *[]) {cursor.type, result}[cursor.charge], !cursor.charge
        };
        Attitude target = {
          (MuonType *[]) {result, cursor.type}[cursor.charge], !cursor.charge
        };

        Rule *rule;
        if ((rule = rule_insert(inductor, source.type, target.type)) == NULL)
          goto except;
        rule->tag = INSTANCE_RULE;
        rule->source = attitude_encode(source);
        rule->target = attitude_encode(target);
        rule->instance_scheme = scheme;
        rule->instance_id = instance_id;
        break;
    }

    cursor = type_return(next = cursor);
    series = type_attach(series, next);
  } while (!attitude_isnull(cursor));

  while (!attitude_isnull(type_detach(series))) {}

  MuonType *result = equation[scheme->matter->id].result;
  return assert(result != NULL), result;

except:
  while (!attitude_isnull(type_detach(series))) {}
  while (!attitude_isnull(cursor = type_return(cursor))) {}
  return NULL;
}
