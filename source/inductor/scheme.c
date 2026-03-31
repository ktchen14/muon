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

  struct MuonInstance *instance_allocation;
  if ((instance_allocation = instance_allocate(engine, scheme)) == NULL)
    return NULL;

  for (size_t i = 0; i < scheme->argc; i++) {
    MuonImplicitType *result;
    if ((result = muon_implicit_type(engine)) == NULL)
      return NULL;
    instance_allocation->argv[i] = result;
  }

  MuonInstance *instance;
  if ((instance = instance_activate(instance_allocation)) == NULL)
    return NULL;

  inductor->instance[inductor->instance_length++] = instance;

  MuonType *equation[inductor->type_length] = {};
  for (size_t i = 0; i < scheme->argc; i++)
    equation[scheme->argv[i]->as_type.id] = &instance->argv[i]->as_type;

  // Used to mark each returned attitude as accessible
  Attitude series = {(MuonType *) &(TypeHeader) {}.type, 0};

  Attitude cursor = {scheme->matter, 0};
  do {
    Attitude next;
    while (!attitude_isnull(next = type_next2(inductor, cursor))) {
      struct TypeCursor *next_cursor = type_cursor(next);
      if (!attitude_isnull(attitude_decode(next_cursor->anterior)))
        continue;
      if (next_cursor->i != 0)
        continue;

      if (next.type->scheme != NULL && next.type->scheme != scheme
          && equation[next.type->scheme->as_type.id] == NULL)
        continue;

      cursor = type_continue(cursor, next);
    }

    // TODO: fix nested scheme handling. We probably need separate
    // implicit_type_allocate/implicit_type_activate functions.

    switch ON_ABSTRACT_TYPE(cursor.type) {
      case IS_CONCRETE_TYPE(MuonCoreType *core_type) {
        MuonCore *core = core_type->core;

        struct MuonCoreType *allocation;
        if ((allocation = core_type_allocate(engine, core)) == NULL)
          goto except;

        for (size_t i = 0; i < core_argc(core); i++) {
          MuonCoreMember member = core_at(core, i);
          MuonType *argument = core_type->argv[member.i];
          if (equation[argument->id] != NULL)
            argument = equation[argument->id];
          allocation->argv[i] = argument;
        }

        MuonCoreType *result;
        if ((result = core_type_activate(allocation)) == NULL)
          goto except;
        equation[cursor.type->id] = &result->as_type;
        break;
      }

      case MUON_IMPLICIT_TYPE:
        break;

      case IS_CONCRETE_TYPE(MuonJoinType *join_type) {
        struct MuonJoinType *allocation;
        if ((allocation = join_type_allocate(engine, join_type->argc)) == NULL)
          goto except;

        for (size_t i = 0; i < join_type->argc; i++) {
          MuonType *argument = join_type->argv[i];
          if (equation[argument->id] != NULL)
            argument = equation[argument->id];
          allocation->argv[i] = argument;
        }

        MuonJoinType *result;
        if ((result = join_type_activate(allocation)) == NULL)
          goto except;
        equation[cursor.type->id] = &result->as_type;
        break;
      }

      case IS_CONCRETE_TYPE(MuonMeetType *meet_type) {
        struct MuonMeetType *allocation;
        if ((allocation = meet_type_allocate(engine, meet_type->argc)) == NULL)
          goto except;

        for (size_t i = 0; i < meet_type->argc; i++) {
          MuonType *argument = meet_type->argv[i];
          if (equation[argument->id] != NULL)
            argument = equation[argument->id];
          allocation->argv[i] = argument;
        }

        MuonMeetType *result;
        if ((result = meet_type_activate(allocation)) == NULL)
          goto except;
        equation[cursor.type->id] = &result->as_type;
        break;
      }

      case IS_CONCRETE_TYPE(MuonSchemeType *scheme_type) {
        // struct MuonSchemeType *allocation;
            // (struct MuonSchemeType *) equation[cursor.type->id].allocation;
        //
        // MuonType *matter = scheme_type->matter;
        // if (equation[matter->id].result != NULL)
        //   matter = equation[matter->id].result;
        // allocation->matter = matter;
        //
        // MuonSchemeType *result;
        // if ((result = scheme_type_activate(allocation)) == NULL)
        //   goto except;
        // equation[cursor.type->id].result = &result->as_type;
        break;
      }

      case IS_CONCRETE_TYPE(MuonVariableType *variable_type) {
        MuonType *result = equation[cursor.type->id];

        if (cursor.charge == 0) {
          if (!is_bottom_type(variable_type->join)) {
            MuonType *join = variable_type->join;
            if (equation[join->id] != NULL)
              join = equation[join->id];
            rule_insert(inductor, join, result);
          }
        } else {
          if (!is_object_type(variable_type->meet)) {
            MuonType *meet = variable_type->meet;
            if (equation[meet->id] == NULL)
              meet = equation[meet->id];
            rule_insert(inductor, result, meet);
          }
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

  MuonType *result = equation[scheme->matter->id];
  return assert(result != NULL), result;

except:
  while (!attitude_isnull(series = type_return(series))) {}
  while (!attitude_isnull(cursor = type_return(cursor))) {}
  return NULL;
}
