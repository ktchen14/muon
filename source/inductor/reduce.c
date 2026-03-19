#include "reduce.h"

#include "common.h"
#include "induce.h"
#include "node.h"

#include <assert.h>
#include <errno.h>
#include <stdckdint.h>
#include <stdlib.h>

[[gnu::nonnull, gnu::pure]]
static inline AttitudeSolution *attitude_solution_get(
    const Inductor *inductor, Attitude attitude) {
  assert(attitude.type->id < inductor->type_length);
  return inductor->attitude_solution[attitude.type->id * 2 + attitude.charge];
}

[[gnu::nonnull]]
static inline void attitude_solution_set(
    Inductor *inductor, Attitude attitude, AttitudeSolution *solution) {
  assert(attitude.type->id < inductor->type_length);
  inductor->attitude_solution[attitude.type->id * 2 + attitude.charge] =
      solution;
}

static inline MuonType *assign_solution(
    Inductor *inductor, MuonType *type, MuonType *solution) {
  assert(type->id < inductor->type_length);
  return inductor->solution[type->id] = solution;
}

/// Look up the solution for a constraint neighbor of a variable at a given
/// charge.  All neighbors must already have been reduced by the traversal.
///
/// For nonvariable neighbors: returns the full (overall) solution.
/// For variable neighbors: returns the attitude-specific base at the same
/// charge, or the full solution if already resolved.
static inline MuonType *neighbor_solution(
    const Inductor *inductor, MuonType *neighbor, _Bool charge) {
  MuonType *sol = type_solution(inductor, neighbor);
  if (sol != NULL)
    return sol;

  if (neighbor->tag == MUON_VARIABLE_TYPE) {
    AttitudeSolution *att = attitude_solution_get(
        inductor, (Attitude) {neighbor, charge});
    assert(att != NULL);
    return att->base;
  }

  // Nonvariable neighbor must have a full solution by now.
  assert(0 && "nonvariable neighbor has no solution");
  return NULL;
}

/// Compute the attitude-specific solution for a variable type at a given
/// charge direction.
///
/// All constraint neighbors must already have been reduced by the traversal.
/// This function only reads pre-computed solutions — it does not trigger any
/// further reductions.
static MuonType *reduce_variable_attitude(
    Inductor *inductor, MuonVariableType *target, _Bool charge) {
  AttitudeSolution *existing = attitude_solution_get(
      inductor, (Attitude) {&target->as_type, charge});
  if (existing != NULL)
    return existing->base;

  Attitude target_attitude = {&target->as_type, charge};

  // Phase 1 — count the join length from non-instance neighbor solutions.
  size_t argc = 0;
  RuleIterator it;

  it = rule_iterator(inductor, target_attitude);
  for (Rule *rule; (rule = rule_next(&it)) != NULL;) {
    if (rule->tag == INDIRECT_RULE || rule->tag == INSTANCE_RULE
        || rule->tag == FORWARDED_RULE)
      continue;

    Attitude source_att = attitude_decode(rule->vertex[charge]);
    MuonType *sol = neighbor_solution(inductor, source_att.type, charge);
    assert(sol != NULL);

    MuonJoinType *jt;
    if ((jt = muon_type_cast(sol, jt)) != NULL) {
      if (rare(ckd_add(&argc, argc, jt->argc)))
        return errno = ENOMEM, NULL;
    } else {
      if (rare(ckd_add(&argc, argc, 1)))
        return errno = ENOMEM, NULL;
    }
  }

  // Phase 2 — pairwise coercion elimination.
  Rule *single_edge;
  MuonType *single_a;
  argc = 0;
  it = rule_iterator(inductor, target_attitude);
  for (Rule *a_edge; (a_edge = rule_next(&it)) != NULL;) {
    if (a_edge->tag == INDIRECT_RULE || a_edge->tag == INSTANCE_RULE
        || a_edge->tag == FORWARDED_RULE)
      continue;
    Attitude a_att = attitude_decode(a_edge->vertex[charge]);
    MuonType *a = neighbor_solution(inductor, a_att.type, charge);
    assert(a != NULL);

    RuleIterator jt = it;
    for (Rule *b_edge; (b_edge = rule_next(&jt)) != NULL;) {
      if (b_edge->tag == INDIRECT_RULE || b_edge->tag == INSTANCE_RULE
          || b_edge->tag == FORWARDED_RULE)
        continue;
      Attitude b_att = attitude_decode(b_edge->vertex[charge]);
      MuonType *b = neighbor_solution(inductor, b_att.type, charge);
      assert(b != NULL);

      const Rule *b_to_a;
      if ((b_to_a = type_assess(inductor, b, a)) == NULL)
        return NULL;
      if (b_to_a->tag != IMPOSSIBLE_RULE) {
        b_edge->tag = INDIRECT_RULE;
        b_edge->center = a;
        continue;
      }

      const Rule *a_to_b;
      if ((a_to_b = type_assess(inductor, a, b)) == NULL)
        return NULL;
      if (a_to_b->tag != IMPOSSIBLE_RULE) {
        a_edge->tag = INDIRECT_RULE;
        a_edge->center = b;
        goto continue_a;
      }
    }

    single_a = a;
    single_edge = a_edge;
    argc++;
  continue_a:;
  }

  // Phase 3 — produce the join result (the attitude-specific solution).
  MuonType *join_result;

  if (argc == 1) {
    join_result = single_a;
  } else if (argc == 0) {
    join_result = &as_engine(inductor->engine)->bottom_type->as_type;
  } else {
    struct MuonJoinType *allocation;
    if ((allocation = join_type_allocate(inductor->engine, argc)) == NULL)
      return NULL;
    argc = 0;

    it = rule_iterator(inductor, target_attitude);
    for (Rule *edge; (edge = rule_next(&it)) != NULL;) {
      if (edge->tag == INDIRECT_RULE || edge->tag == INSTANCE_RULE
          || edge->tag == FORWARDED_RULE)
        continue;

      Attitude source_att = attitude_decode(edge->vertex[charge]);
      MuonType *source = neighbor_solution(inductor, source_att.type, charge);
      assert(source != NULL);

      allocation->argv[argc] = source;

      Rule *rule;
      if ((rule = edge_define(inductor, source, &allocation->as_type)) == NULL)
        return NULL;
      rule->tag = JOIN_RULE;
      rule->i = argc++;
    }
    assert(argc == allocation->argc);

    MuonJoinType *join_type;
    if ((join_type = join_type_activate(allocation)) == NULL)
      return NULL;

    join_result = &join_type->as_type;
  }

  // Phase 4 — build the AttitudeSolution.
  size_t instance_argc = 0;
  it = rule_iterator(inductor, target_attitude);
  for (Rule *rule; (rule = rule_next(&it)) != NULL;) {
    if (rule->tag == INDIRECT_RULE || rule->tag == FORWARDED_RULE)
      continue;
    if (rule->instance != NULL) {
      Attitude nbr = attitude_decode(rule->vertex[charge]);
      MuonType *nbr_sol = type_solution(inductor, nbr.type);
      if (nbr_sol == NULL) {
        AttitudeSolution *nbr_att = attitude_solution_get(
            inductor, (Attitude) {nbr.type, charge});
        if (nbr_att != NULL)
          nbr_sol = nbr_att->base;
      }
      if (nbr_sol != NULL)
        instance_argc++;
    }
  }

  size_t size = instance_argc;
  if (struct_size_overflow(AttitudeSolution, argv, &size))
    return errno = ENOMEM, NULL;
  AttitudeSolution *att_sol;
  if ((att_sol = malloc(size)) == NULL)
    return NULL;

  att_sol->base = join_result;
  att_sol->argc = instance_argc;

  size_t j = 0;
  it = rule_iterator(inductor, target_attitude);
  for (Rule *rule; (rule = rule_next(&it)) != NULL;) {
    if (rule->tag == INDIRECT_RULE || rule->tag == FORWARDED_RULE)
      continue;
    if (rule->instance != NULL) {
      Attitude nbr = attitude_decode(rule->vertex[charge]);
      MuonType *nbr_sol = type_solution(inductor, nbr.type);
      if (nbr_sol == NULL) {
        AttitudeSolution *nbr_att = attitude_solution_get(
            inductor, (Attitude) {nbr.type, charge});
        if (nbr_att != NULL)
          nbr_sol = nbr_att->base;
      }
      if (nbr_sol != NULL) {
        att_sol->argv[j].instance = rule->instance;
        att_sol->argv[j].type = nbr_sol;
        j++;
      }
    }
  }
  assert(j == instance_argc);

  attitude_solution_set(inductor, target_attitude, att_sol);

  // Phase 5 — create FORWARDED_RULE edges from the AttitudeSolution.
  //
  // For each downstream type t (found via rule_scan at the opposite charge),
  // create new rules from the AttitudeSolution's base and per-instance types
  // to t.  These rules are tagged FORWARDED_RULE and don't affect existing
  // code paths (which skip FORWARDED_RULE).
  Attitude scan_attitude = {&target->as_type, !charge};
  it = rule_iterator(inductor, scan_attitude);
  for (Rule *target_rule; (target_rule = rule_scan(&it)) != NULL;) {
    if (target_rule->tag == INDIRECT_RULE || target_rule->tag == FORWARDED_RULE)
      continue;

    MuonType *t = attitude_decode(target_rule->vertex[!charge]).type;

    // Forward the base join to t.
    {
      MuonType *src = charge == 0 ? att_sol->base : t;
      MuonType *tgt = charge == 0 ? t : att_sol->base;
      Rule *fwd = rule_insert(inductor, src, tgt);
      if (fwd == NULL)
        return NULL;
      fwd->tag = FORWARDED_RULE;
      fwd->instance = target_rule->instance;
    }

    // Forward each per-instance type to t.
    for (size_t k = 0; k < att_sol->argc; k++) {
      MuonType *inst_type = att_sol->argv[k].type;
      MuonType *src = charge == 0 ? inst_type : t;
      MuonType *tgt = charge == 0 ? t : inst_type;
      Rule *fwd = rule_insert(inductor, src, tgt);
      if (fwd == NULL)
        return NULL;
      fwd->tag = FORWARDED_RULE;
      // The instance comes from the AttitudeSolution entry, but if the
      // target rule itself has an instance (e.g., INSTANCE_RULE), the
      // base partition should inherit that instance.
      fwd->instance = att_sol->argv[k].instance;
    }
  }

  return join_result;
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
        if (reduce_variable_attitude(inductor, variable_type, cursor.charge)
            == NULL)
          return NULL;

        AttitudeSolution *pos = attitude_solution_get(
            inductor, (Attitude) {&variable_type->as_type, 0});
        AttitudeSolution *neg = attitude_solution_get(
            inductor, (Attitude) {&variable_type->as_type, 1});
        if (pos != NULL && neg != NULL) {
          MuonType *overall = pos->base;
          assign_solution(inductor, &variable_type->as_type, overall);
        }
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

    if (attitude_solution_get(inductor, next = attitude_invert(next)) != NULL)
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
