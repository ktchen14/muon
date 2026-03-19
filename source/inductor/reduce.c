#include "reduce.h"

#include "common.h"
#include "induce.h"
#include "node.h"

#include <assert.h>
#include <errno.h>
#include <stdckdint.h>
#include <stdlib.h>

static inline MuonType *assign_solution(
    Inductor *inductor, MuonType *type, MuonType *solution) {
  assert(type->id < inductor->type_length);
  debug("Assigning type %zu, i.e. ", type->id);
  muon_type_debug(type);
  debug(" solution: ");
  muon_type_debug(solution);
  debug("\n");
  return inductor->solution[type->id] = solution;
}

[[gnu::nonnull, gnu::pure]]
static inline _Bool attitude_is_done(
    const Inductor *inductor, Attitude attitude) {
  assert(attitude.type->id < inductor->type_length);
  return inductor->attitude_done[attitude.type->id * 2 + attitude.charge];
}

[[gnu::nonnull]]
static inline void attitude_mark_done(Inductor *inductor, Attitude attitude) {
  assert(attitude.type->id < inductor->type_length);
  inductor->attitude_done[attitude.type->id * 2 + attitude.charge] = 1;
}

#include <stdio.h>
#include "../inductor.h"

/// Compute the attitude-specific solution for a variable type at a given
/// charge direction.
///
/// All constraint neighbors must already have been reduced by the traversal.
/// This function only reads pre-computed solutions — it does not trigger any
/// further reductions.
static MuonType *reduce_variable_type_0(
    Inductor *inductor, MuonVariableType *variable_type, _Bool charge) {
  assert(charge == 0);
  MuonType *type = &variable_type->as_type;

  Attitude attitude = {type, charge};
  if (attitude_is_done(inductor, attitude))
    return type;

  RuleIterator it;

  MuonType *single;
  size_t argc = 0;
  it = rule_iterator(inductor, (Attitude) {type, charge});
  for (Rule *edge; (edge = rule_next(&it)) != NULL;) {
    if (edge->tag == INDIRECT_RULE)
      continue;
    if (edge->instance != NULL)
      continue;
    single = edge->vertex[charge];
    argc++;
  }

  MuonType *result;

  if (argc == 0)
    result = &as_engine(inductor->engine)->bottom_type->as_type;
  else if (argc == 1)
    result = single;
  else {
    struct MuonJoinType *allocation;
    if ((allocation = join_type_allocate(inductor->engine, argc)) == NULL)
      return NULL;

    it = rule_iterator(inductor, (Attitude) {type, charge});
    size_t i = 0;
    for (Rule *edge; (edge = rule_next(&it)) != NULL;) {
      if (edge->tag == INDIRECT_RULE)
        continue;
      if (edge->instance != NULL)
        continue;

      MuonType *solution = type_solution(inductor, edge->vertex[charge]);
      assert(solution != NULL);
      allocation->argv[i++] = solution;
    }
    assert(i == argc);

    MuonJoinType *join_type;
    if ((join_type = join_type_activate(allocation)) == NULL)
      return NULL;

    // For each type τ, ... in Join(τ, ...), make ⟨τ ⇒ Join(τ, ...)⟩ and make
    // ⟨τ ⇒ type⟩ indirect through the join type.
    it = rule_iterator(inductor, (Attitude) {type, charge});
    i = 0;
    for (Rule *edge; (edge = rule_next(&it)) != NULL;) {
      if (edge->tag == INDIRECT_RULE)
        continue;
      if (edge->instance != NULL)
        continue;

      MuonType *argument = join_type->argv[i++];
      assert(argument == type_solution(inductor, edge->vertex[charge]));

      edge->tag = INDIRECT_RULE;
      edge->center = &join_type->as_type;

      Rule *rule;
      if ((rule = edge_define(inductor, argument, &join_type->as_type)) == NULL)
        return NULL;
      rule->tag = JOIN_RULE;
      rule->i = i;
    }

    result = &join_type->as_type;
  }

  edge_define(inductor, result, type);

  // Phase 5 — create FORWARDED_RULE edges from the AttitudeSolution.
  //
  // For each downstream type t (found via rule_scan at the opposite charge),
  // create new rules from the AttitudeSolution's base and per-instance types
  // to t.  These rules are tagged FORWARDED_RULE and don't affect existing
  // code paths (which skip FORWARDED_RULE).
  it = rule_iterator(inductor, (Attitude) {type, !charge});
  for (Rule *rule; (rule = rule_scan(&it)) != NULL;) {
    if (rule->tag == INDIRECT_RULE)
      continue;
    if (!is_variable_type(rule->vertex[!charge]))
      continue;

    rule->locked[charge] = 1;
    rule->center = result;

    MuonType *next = rule->vertex[!charge];
    Rule *newrule;
    if ((newrule = edge_define(inductor, result, next)) == NULL)
      return NULL;
    newrule->instance = rule->instance;
  }

  attitude_mark_done(inductor, attitude);
  return type;
}

static MuonType *reduce_variable_type_1(
    Inductor *inductor, MuonVariableType *variable_type, _Bool charge) {
  assert(charge == 1);
  MuonType *type = &variable_type->as_type;

  Attitude attitude = {type, 1};
  if (attitude_is_done(inductor, attitude))
    return type;

  RuleIterator it;

  MuonType *single;
  size_t argc = 0;
  it = rule_iterator(inductor, (Attitude) {type, 1});
  for (Rule *edge; (edge = rule_next(&it)) != NULL;) {
    if (edge->tag == INDIRECT_RULE)
      continue;
    if (edge->instance != NULL)
      continue;
    single = edge->vertex[charge];
    argc++;
  }

  MuonType *result;

  if (argc == 0)
    result = &as_engine(inductor->engine)->object_type->as_type;
  else if (argc == 1)
    result = single;
  else {
    struct MuonMeetType *allocation;
    if ((allocation = meet_type_allocate(inductor->engine, argc)) == NULL)
      return NULL;

    it = rule_iterator(inductor, (Attitude) {type, 1});
    size_t i = 0;
    for (Rule *edge; (edge = rule_next(&it)) != NULL;) {
      if (edge->tag == INDIRECT_RULE)
        continue;
      if (edge->instance != NULL)
        continue;
      MuonType *solution = type_solution(inductor, edge->vertex[charge]);
      assert(solution != NULL);
      allocation->argv[i++] = solution;
    }
    assert(i == argc);

    MuonMeetType *meet_type;
    if ((meet_type = meet_type_activate(allocation)) == NULL)
      return NULL;

    // For each type τ, ... in Meet(τ, ...), make ⟨τ ⇒ Meet(τ, ...)⟩ and make
    // ⟨τ ⇒ type⟩ indirect through the join type.
    it = rule_iterator(inductor, (Attitude) {type, 1});
    i = 0;
    for (Rule *edge; (edge = rule_next(&it)) != NULL;) {
      if (edge->tag == INDIRECT_RULE)
        continue;
      if (edge->instance != NULL)
        continue;

      MuonType *argument = meet_type->argv[i++];
      assert(argument == type_solution(inductor, edge->vertex[1]));

      edge->tag = INDIRECT_RULE;
      edge->center = &meet_type->as_type;

      Rule *rule;
      if ((rule = edge_define(inductor, &meet_type->as_type, argument)) == NULL)
        return NULL;
      rule->tag = JOIN_RULE;
      rule->i = i;
    }

    result = &meet_type->as_type;
  }

  edge_define(inductor, type, result);

  // Phase 5 — create FORWARDED_RULE edges from the AttitudeSolution.
  //
  // For each downstream type t (found via rule_scan at the opposite charge),
  // create new rules from the AttitudeSolution's base and per-instance types
  // to t.  These rules are tagged FORWARDED_RULE and don't affect existing
  // code paths (which skip FORWARDED_RULE).
  it = rule_iterator(inductor, (Attitude) {type, 0});
  for (Rule *rule; (rule = rule_scan(&it)) != NULL;) {
    if (rule->tag == INDIRECT_RULE)
      continue;
    if (!is_variable_type(rule->vertex[!charge]))
      continue;

    rule->locked[charge] = 1;
    rule->center = result;

    MuonType *next = rule->vertex[0];
    Rule *newrule;
    if ((newrule = edge_define(inductor, next, result)) == NULL)
      return NULL;
    newrule->instance = rule->instance;
  }

  attitude_mark_done(inductor, attitude);
  return type;
}

/// Reduce a type to its solution.
///
/// Performs an iterative depth-first traversal of the type tree using
/// type_next / type_continue / type_return.  On return from each type:
///
///   - Variable types: compute the attitude-specific solution for the
///     current charge direction via reduce_variable_attitude.
///
///   - Nonvariable types: resolve structural variable children that now
///     have both attitude solutions, then reconstruct the type.
///
/// When returning from a variable child to a nonvariable parent, if the
/// variable child does not yet have an AttitudeSolution at the inverted
/// charge, re-enter the variable at the inverted charge.  When it does,
/// resolve the variable's overall solution immediately.
MuonType *reduce_type(Inductor *inductor, Attitude attitude) {
  MuonType *solution;
  if ((solution = type_solution(inductor, attitude.type)) != NULL)
    return solution;

  MuonEngine *engine = inductor->engine;

  Attitude cursor = attitude;
  do {
    Attitude next;
    while (!attitude_isnull(next = type_next(inductor, cursor))) {
      struct TypeCursor *next_cursor = type_cursor(next);
      if (!attitude_isnull(attitude_decode(next_cursor->anterior)))
        continue;
      if (next_cursor->i != 0)
        continue;

      if (type_solution(inductor, next.type) != NULL)
        continue;

      cursor = type_continue(cursor, next);
    }

    // On return, create the solution of the returned type. If the type is a
    // variable type, then this is a single charge solution. Otherwise, this is
    // a universal solution.
    switch ON_ABSTRACT_TYPE(cursor.type) {
      case IS_CONCRETE_TYPE(MuonCoreType *core_type) {
        MuonCore *core = core_type->core;

        struct MuonCoreType *allocation;
        if ((allocation = core_type_allocate(engine, core)) == NULL)
          return NULL;
        for (size_t i = 0; i < core_argc(core); i++) {
          size_t j = core_at(core, i).i;
          MuonType *argument = type_solution(inductor, core_type->argv[j]);
          assert(argument != NULL);
          allocation->argv[i] = argument;
        }
        MuonCoreType *result;
        if ((result = core_type_activate(allocation)) == NULL)
          return NULL;
        assign_solution(inductor, cursor.type, &result->as_type);
        break;
      }

      case IS_CONCRETE_TYPE(MuonJoinType *join_type) {
        struct MuonJoinType *allocation;
        if ((allocation = join_type_allocate(engine, join_type->argc)) == NULL)
          return NULL;
        for (size_t i = 0; i < join_type->argc; i++) {
          MuonType *argument = type_solution(inductor, join_type->argv[i]);
          assert(argument != NULL);
          allocation->argv[i] = argument;
        }
        MuonJoinType *result;
        if ((result = join_type_activate(allocation)) == NULL)
          return NULL;
        assign_solution(inductor, cursor.type, &result->as_type);
        break;
      }

      case IS_CONCRETE_TYPE(MuonMeetType *meet_type) {
        struct MuonMeetType *allocation;
        if ((allocation = meet_type_allocate(engine, meet_type->argc)) == NULL)
          return NULL;
        for (size_t i = 0; i < meet_type->argc; i++) {
          MuonType *argument = type_solution(inductor, meet_type->argv[i]);
          assert(argument != NULL);
          allocation->argv[i] = argument;
        }
        MuonMeetType *result;
        if ((result = meet_type_activate(allocation)) == NULL)
          return NULL;
        assign_solution(inductor, cursor.type, &result->as_type);
        break;
      }

      case IS_CONCRETE_TYPE(MuonSchemeType *scheme_type) {
        // TODO: Scheme types may need special handling in the future.
        MuonType *matter = scheme_type->matter;
        matter = type_solution(inductor, matter);
        assert(matter != NULL);
        assign_solution(inductor, cursor.type, matter);
        break;
      }

      case IS_CONCRETE_TYPE(MuonVariableType *variable_type) {
        debug("Reducing v%zu charge = %d\n", cursor.type->id, cursor.charge);
        if (cursor.charge == 0) {
          if (reduce_variable_type_0(inductor, variable_type, cursor.charge)
              == NULL)
            return NULL;
        } else {
          if (reduce_variable_type_1(inductor, variable_type, cursor.charge)
              == NULL)
            return NULL;
        }

        if (!attitude_is_done(inductor, cursor) || !attitude_is_done(inductor, attitude_invert(cursor)))
          break;

        _Bool solution_charge = 0;
        RuleIterator it = rule_iterator(inductor, (Attitude) {cursor.type, solution_charge});
        for (Rule *edge; (edge = rule_next(&it)) != NULL;) {
          if (edge->tag == INDIRECT_RULE)
            continue;
          if (edge->instance != NULL)
            continue;
          assign_solution(inductor, cursor.type, edge->vertex[solution_charge]);
          goto done;
        }

        Engine *engine = as_engine(inductor->engine);
        MuonType *solution = &engine->bottom_type->as_type;
        assign_solution(inductor, cursor.type, solution);

      done:
        break;
      }
    }

    cursor = type_return(next = cursor);

    // If we've returned from a variable type to a nonvariable type, then check
    // whether the variable needs its inverted attitude solution. If so,
    // re-enter at the inverted charge. If both attitude solutions already
    // exist, resolve the variable now.
    if (cursor.type != NULL && is_variable_type(cursor.type))
      continue;

    if (!is_variable_type(next.type))
      continue;

    if (attitude_is_done(inductor, next = attitude_invert(next)))
      continue;
    cursor = type_continue(cursor, next);
  } while (!attitude_isnull(cursor));

  return type_solution(inductor, attitude.type);
}

MuonType *reduce_node(Inductor *inductor, MuonNode *root) {
  MuonNode *node = root;
  do {
    MuonNode *next;
    while ((next = node_at(node, node_cursor(node)->i++)) != NULL)
      node = node_continue(node, next);

    // 1. Reduce the source type
    MuonType *source_type = node_source_type(inductor, node);
    reduce_type(inductor, (Attitude) {source_type, 0});

    // 2. If the node has a target type, reduce it too
    MuonType *target_type = node_target_type(inductor, node);
    if (target_type != NULL)
      reduce_type(inductor, (Attitude) {target_type, 0});

  } while ((node = node_return(node)) != NULL);

  return node_source_type(inductor, root);
}
