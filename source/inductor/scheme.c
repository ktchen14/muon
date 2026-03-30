#include "scheme.h"

#include "common.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static inline Attitude type_next2(const Inductor *inductor, Attitude origin) {
  return type_at(origin, type_cursor(origin)->i++);
}

MuonType *scheme_instance(MuonInductor *inductor, MuonSchemeType *scheme) {
  MuonEngine *engine = inductor->engine;

  MuonInstance *instance;
  if ((instance = muon_instance(engine, scheme)) == NULL)
    return NULL;

  union {
    struct MuonType *allocation;
    MuonType *result;
  } equation[inductor->type_length] = {};

  // Used to mark each returned attitude as accessible
  Attitude series = {(MuonType *) &(TypeHeader) {}.type, 0};

  Attitude cursor = {scheme->matter, 0};
  goto entrance;
  do {
    Attitude next;
    while (!attitude_isnull(next = type_next2(inductor, cursor))) {
      struct TypeCursor *next_cursor = type_cursor(next);
      if (!attitude_isnull(attitude_decode(next_cursor->anterior)))
        continue;
      if (next_cursor->i != 0)
        continue;

      if (next.type->scheme != NULL && next.type->scheme != scheme
          && equation[next.type->scheme->as_type.id].allocation == NULL)
        continue;

      cursor = type_continue(cursor, next);

      if (equation[cursor.type->id].allocation != NULL)
        continue;

    entrance:
      switch ON_ABSTRACT_TYPE(cursor.type) {
        case IS_CONCRETE_TYPE(MuonCoreType *core_type) {
          MuonCore *core = core_type->core;

          struct MuonCoreType *allocation;
          if ((allocation = core_type_allocate(engine, core)) == NULL)
            goto except;
          equation[cursor.type->id].allocation = &allocation->as_type;
          break;
        }

        case MUON_IMPLICIT_TYPE:
        case MUON_VARIABLE_TYPE: {
          MuonImplicitType *result;
          if ((result = muon_implicit_type(engine)) == NULL)
            goto except;
          equation[cursor.type->id].result = &result->as_type;
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
          // struct MuonSchemeType *allocation;
          // if ((allocation = scheme_type_allocate(engine)) == NULL)
          //   goto except;
          // equation[cursor.type->id].allocation = &allocation->as_type;
          break;
        }
      }
    }

    // TODO: fix nested scheme handling. We probably need separate
    // implicit_type_allocate/implicit_type_activate functions.

    switch ON_ABSTRACT_TYPE(cursor.type) {
      case IS_CONCRETE_TYPE(MuonCoreType *core_type) {
        struct MuonCoreType *allocation =
            (struct MuonCoreType *) equation[cursor.type->id].allocation;

        MuonCore *core = core_type->core;

        for (size_t i = 0; i < core_argc(core); i++) {
          MuonCoreMember member = core_at(core, i);
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

      case MUON_IMPLICIT_TYPE: {
        RuleIterator it = rule_iterator(inductor, cursor);
        for (const Rule *rule; (rule = rule_next(&it)) != NULL;) {
          if (rule->instance != NULL && rule->instance->scheme == scheme)
            continue;

          MuonType *source = rule->source;
          if (equation[source->id].result != NULL)
            source = equation[source->id].result;

          MuonType *target = rule->target;
          if (equation[target->id].result != NULL)
            target = equation[target->id].result;

          if (rule_search(inductor, source, target) != NULL)
            continue;

          Rule *next;
          if ((next = rule_insert(inductor, source, target)) == NULL)
            goto except;
          next->tag = rule->tag;
          memcpy(next->locked, rule->locked, sizeof(next->locked));
          next->instance = rule->instance;
        }

        MuonType *result = equation[cursor.type->id].result;

        MuonType *source = (MuonType *[]) {cursor.type, result}[cursor.charge];
        MuonType *target = (MuonType *[]) {result, cursor.type}[cursor.charge];

        Rule *rule;
        if ((rule = rule_insert(inductor, source, target)) == NULL)
          goto except;
        rule->locked[cursor.charge] = 1;
        rule->instance = instance;
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
        allocation->matter = matter;

        MuonSchemeType *result;
        if ((result = scheme_type_activate(allocation)) == NULL)
          goto except;
        equation[cursor.type->id].result = &result->as_type;
        break;
      }

      case IS_CONCRETE_TYPE(MuonVariableType *variable_type) {
        MuonType *result = equation[cursor.type->id].result;

        if (cursor.charge == 0) {
          if (!is_bottom_type(variable_type->join))
            rule_insert(inductor, variable_type->join, result);
        } else {
          if (!is_object_type(variable_type->meet))
            rule_insert(inductor, result, variable_type->meet);
        }

        MuonType *source = (MuonType *[]) {cursor.type, result}[cursor.charge];
        MuonType *target = (MuonType *[]) {result, cursor.type}[cursor.charge];

        Rule *rule;
        if ((rule = rule_insert(inductor, source, target)) == NULL)
          goto except;
        rule->locked[cursor.charge] = 1;
        rule->instance = instance;
        break;
      }
    }

    cursor = type_return(next = cursor);
    series = type_continue(series, next);
  } while (!attitude_isnull(cursor));

  while (!attitude_isnull(series = type_return(series))) {}

  MuonType *result = equation[scheme->matter->id].result;
  return assert(result != NULL), result;

except:
  while (!attitude_isnull(series = type_return(series))) {}
  while (!attitude_isnull(cursor = type_return(cursor))) {}
  return NULL;
}
