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
  return inductor->solution[type->id] = solution;
}

/// Compute the overall solution for a variable type from its two
/// attitude-specific solutions.  Kept as a separate function so that the
/// resolution strategy can be enhanced later.
static MuonType *resolve_variable(
    Inductor *inductor, MuonVariableType *variable) {
  MuonType *solution;
  if ((solution = type_solution(inductor, &variable->as_type)) != NULL)
    return solution;

  Solution *pos = attitude_solution_get(
      inductor, (Attitude) {&variable->as_type, 0});
  Solution *neg = attitude_solution_get(
      inductor, (Attitude) {&variable->as_type, 1});
  assert(pos != NULL && neg != NULL);

  MuonType *overall = pos->type;
  assert(overall != NULL);
  return assign_solution(inductor, &variable->as_type, overall);
}

/// Look up the solution for a constraint neighbor of a variable at a given
/// charge.  All neighbors must already have been reduced by the traversal.
///
/// For nonvariable neighbors: returns the full (overall) solution.
/// For variable neighbors: returns the attitude-specific base at the same
/// charge, or the full solution if already resolved.
static inline MuonType *neighbor_solution(
    const Inductor *inductor, MuonType *type, _Bool charge) {
  MuonType *result;
  if ((result = type_solution(inductor, type)) != NULL)
    return result;

  assert(is_variable_type(type));

  Attitude attitude = {type, charge};
  Solution *solution = attitude_solution_get(inductor, attitude);
  assert(solution != NULL);
  return solution->type;
}

/// Compute the attitude-specific solution for a variable type at a given
/// charge direction.
///
/// All constraint neighbors must already have been reduced by the traversal.
/// This function only reads pre-computed solutions — it does not trigger any
/// further reductions.
static Solution *reduce_variable_attitude(
    Inductor *inductor, MuonVariableType *target, _Bool charge) {
  debug("Reducing ");
  muon_type_debug(&target->as_type);
  debug(" charge = %d\n", charge);

  Attitude attitude = {&target->as_type, charge};

  Solution *solution;
  if ((solution = attitude_solution_get(inductor, attitude)) != NULL)
    return solution;

  RuleIterator it;

  [[maybe_unused]] MuonType *single;
  size_t argc = 0;
  it = rule_iterator(inductor, attitude);
  for (Rule *edge; (edge = rule_next(&it)) != NULL;) {
    if (edge->tag == INDIRECT_RULE)
      continue;
    if (edge->instance != NULL)
      continue;
    single = edge->vertex[charge];
    argc++;
  }

  // Phase 3 — produce the join result (the attitude-specific solution).
  MuonType *join_result;

  if (argc == 0) {
    join_result = &as_engine(inductor->engine)->bottom_type->as_type;
  } else {
    struct MuonJoinType *allocation;
    if ((allocation = join_type_allocate(inductor->engine, argc)) == NULL)
      return NULL;

    it = rule_iterator(inductor, attitude);
    size_t i = 0;
    for (Rule *edge; (edge = rule_next(&it)) != NULL;) {
      if (edge->tag == INDIRECT_RULE)
        continue;
      if (edge->instance != NULL)
        continue;

      MuonType *source = neighbor_solution(
          inductor, edge->vertex[charge], charge);
      assert(source != NULL);

      allocation->argv[i++] = source;
    }
    assert(argc == allocation->argc);

    MuonJoinType *join_type;
    if ((join_type = join_type_activate(allocation)) == NULL)
      return NULL;

    // For each type τ, ... in Join(τ, ...), make ⟨τ ⇒ Join(τ, ...)⟩ and make
    // ⟨τ ⇒ type⟩ indirect through the join type.
    it = rule_iterator(inductor, attitude);
    i = 0;
    for (Rule *edge; (edge = rule_next(&it)) != NULL;) {
      if (edge->tag == INDIRECT_RULE)
        continue;
      if (edge->instance != NULL)
        continue;

      MuonType *argument = join_type->argv[i++];
      assert(argument == neighbor_solution(inductor, edge->vertex[charge], charge));

      edge->tag = INDIRECT_RULE;
      edge->center = &join_type->as_type;

      Rule *rule;
      if ((rule = edge_define(inductor, argument, &join_type->as_type)) == NULL)
        return NULL;
      rule->tag = JOIN_RULE;
      rule->i = i;
    }

    join_result = &join_type->as_type;
  }

  // Phase 4 — build the AttitudeSolution.
  size_t instance_argc = 0;
  it = rule_iterator(inductor, attitude);
  for (Rule *rule; (rule = rule_next(&it)) != NULL;) {
    if (rule->tag == INDIRECT_RULE)
      continue;
    if (rule->instance == NULL)
      continue;

    MuonType *type = rule->vertex[charge];
    MuonType *nbr_sol = type_solution(inductor, type);
    if (nbr_sol == NULL) {
      Solution *solution = attitude_solution_get(
          inductor, (Attitude) {type, charge});
      if (solution != NULL)
        nbr_sol = solution->type;
    }
    if (nbr_sol != NULL)
      instance_argc++;
  }

  size_t size = instance_argc;
  if (struct_size_overflow(Solution, argv, &size))
    return errno = ENOMEM, NULL;
  if ((solution = malloc(size)) == NULL)
    return NULL;

  solution->type = join_result;
  solution->argc = instance_argc;

  size_t j = 0;
  it = rule_iterator(inductor, attitude);
  for (Rule *rule; (rule = rule_next(&it)) != NULL;) {
    if (rule->tag == INDIRECT_RULE)
      continue;

    if (rule->instance == NULL)
      continue;
    MuonType *nbr_sol = type_solution(inductor, rule->vertex[charge]);
    if (nbr_sol == NULL)
      nbr_sol = rule->vertex[charge];
    solution->argv[j].instance = rule->instance;
    solution->argv[j].type = nbr_sol;
    j++;
  }
  assert(j == instance_argc);

  return attitude_solution_set(inductor, attitude, solution);
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
MuonType *reduce_type(Inductor *inductor, MuonType *type) {
  MuonType *solution;
  if ((solution = type_solution(inductor, type)) != NULL)
    return solution;

  MuonEngine *engine = inductor->engine;

  Attitude cursor = {type, 0};
  goto entrance;
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

    entrance:
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
          MuonType *argument = core_type->argv[core_at(core, i).i];
          argument = type_solution(inductor, argument);
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
          MuonType *argument = join_type->argv[i];
          argument = type_solution(inductor, argument);
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
        (void) meet_type;
        assign_solution(inductor, cursor.type, cursor.type);
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
        if (reduce_variable_attitude(inductor, variable_type, cursor.charge)
            == NULL)
          return NULL;

        Attitude invert = attitude_invert(cursor);
        if (attitude_solution_get(inductor, invert) == NULL)
          break;

        resolve_variable(inductor, variable_type);
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

    next = attitude_invert(next);
    if (attitude_solution_get(inductor, next) != NULL)
      continue;

    cursor = type_continue(cursor, next);
  } while (!attitude_isnull(cursor));

  return type_solution(inductor, type);
}

MuonType *reduce_node(Inductor *inductor, MuonNode *root) {
  MuonNode *node = root;
  do {
    MuonNode *next;
    while ((next = node_at(node, node_cursor(node)->i++)) != NULL)
      node = node_continue(node, next);

    // 1. Reduce the source type
    MuonType *source_type = node_source_type(inductor, node);
    reduce_type(inductor, source_type);

    // 2. If the node has a target type, reduce it too
    MuonType *target_type = node_target_type(inductor, node);
    if (target_type != NULL)
      reduce_type(inductor, target_type);

  } while ((node = node_return(node)) != NULL);

  return node_source_type(inductor, root);
}
