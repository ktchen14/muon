#include "reduce.h"

#include "common.h"
#include "induce.h"
#include "node.h"

#include <assert.h>
#include <errno.h>
#include <stdckdint.h>
#include <stdlib.h>

[[gnu::nonnull]] static inline MuonType *assign_solution(
    Inductor *inductor, MuonType *type, MuonType *solution) {
  assert(type->id < inductor->type_length);
  assert(solution->explicit);
  return inductor->solution[type->id] = solution;
}

[[gnu::nonnull]] static Semisolution *assign_semisolution(
    Inductor *inductor, Attitude attitude, Semisolution *solution) {
  assert(attitude.type != NULL && is_implicit_type(attitude.type));
  assert(attitude.type->id < inductor->type_length);
  size_t i = attitude.type->id * 2 + attitude.charge;
  return inductor->semisolution[i] = solution;
}

static Semisolution *type_semisolution(
    const Inductor *inductor, Attitude attitude) {
  assert(attitude.type != NULL && is_implicit_type(attitude.type));
  assert(attitude.type->id < inductor->type_length);
  size_t i = attitude.type->id * 2 + attitude.charge;
  return inductor->semisolution[i];
}

/// Compute the overall solution for an implicit type from its two
/// attitude-specific solutions.  Kept as a separate function so that the
/// resolution strategy can be enhanced later.
static MuonType *implicit_solution(
    Inductor *inductor, Semisolution *semisolution[static 2]) {
  Semisolution *pos = semisolution[0];
  Semisolution *neg = semisolution[1];
  assert(pos != NULL && neg != NULL);

  MuonType *overall = pos->type;
  assert(overall != NULL);
  return overall;
}

/// Compute the attitude-specific solution for an implicit type at a given
/// charge direction.
///
/// All constraint neighbors must already have been reduced by the traversal.
/// This function only reads pre-computed solutions — it does not trigger any
/// further reductions.
static Semisolution *reduce_implicit_type(
    Inductor *inductor, Attitude attitude) {
  assert(is_implicit_type(attitude.type));

  Semisolution *solution;
  if ((solution = type_semisolution(inductor, attitude)) != NULL)
    return solution;

  vector_truncate(inductor->vector, 0);

  RuleIterator it;

  size_t instance_argc = 0;
  it = rule_iterator(inductor, attitude);
  for (const Rule *edge; (edge = rule_next(&it)) != NULL;) {
    if (edge->tag == INDIRECT_RULE)
      continue;

    if (edge->instance != NULL) {
      instance_argc++;
      continue;
    }

    Attitude next = {edge->vertex[attitude.charge], attitude.charge};

    Semisolution *semisolution = &(Semisolution) {};
    MuonType *solution;
    if ((solution = type_solution(inductor, next.type)) != NULL) {
      *semisolution = (Semisolution) {.type = solution};
    } else {
      semisolution = type_semisolution(inductor, next);
      assert(semisolution != NULL && semisolution->type != NULL);
    }

    MuonJoinType *join_type;
    if ((join_type = muon_type_cast(semisolution->type, join_type)) != NULL) {
      for (size_t j = 0; j < join_type->argc; j++) {
        MuonType *argument = join_type->argv[j];

        for (size_t i = 0; i < vector_length(inductor->vector); i++) {
          MuonType *extant = inductor->vector[i];

          // Rule *rule;
          // if ((rule = type_assess(inductor, argument, extant)) == NULL)
          //   abort();

          // if (rule->tag != IMPOSSIBLE_RULE)
          //   goto next_argument;
        }

        Vector(MuonType *) vector;
        if ((vector = vector_append(inductor->vector, &argument)) == NULL)
          abort();
        inductor->vector = vector;
        continue;

      next_argument:
      }
    } else {
      MuonType *solution = semisolution->type;
      for (size_t i = 0; i < vector_length(inductor->vector); i++) {
        MuonType *extant = inductor->vector[i];

        Rule *rule;
        if ((rule = type_assess(inductor, solution, extant)) == NULL)
          return NULL;

        if (rule->tag == REJECTED_RULE)
          continue;

        goto next_rule;
      }

      for (size_t i = 0; i < vector_length(inductor->vector); i++) {
        MuonType *extant = inductor->vector[i];

        Rule *rule;
        if ((rule = type_assess(inductor, extant, solution)) == NULL)
          return NULL;

        if (rule->tag == REJECTED_RULE)
          continue;

        inductor->vector[i] = solution;

        size_t length = vector_length(inductor->vector);

        for (size_t j = i + 1; j < length;) {
          MuonType *extant = inductor->vector[j];

          Rule *rule;
          if ((rule = type_assess(inductor, extant, solution)) == NULL)
            return NULL;

          if (rule->tag == REJECTED_RULE) {
            j++;
            continue;
          }

          inductor->vector[j] = inductor->vector[--length];
        }

        goto next_rule;
      }

      Vector(MuonType *) vector;
      if ((vector = vector_append(inductor->vector, &semisolution->type)) == NULL)
        abort();
      inductor->vector = vector;
    }

  next_rule:
  }

  // Phase 3 — produce the join result (the attitude-specific solution).
  MuonType *join;

  size_t length = vector_length(inductor->vector);
  if (length == 0) {
    join = &as_engine(inductor->engine)->bottom_type->as_type;
  } else if (length == 1) {
    join = inductor->vector[0];
  } else {
    MuonType *const *argv = inductor->vector;
    MuonJoinType *join_type;
    if ((join_type = muon_join_type(inductor->engine, length, argv)) == NULL)
      abort();

    // For each type τ, ... in Join(τ, ...), make ⟨τ ⇒ Join(τ, ...)⟩ and make
    // ⟨τ ⇒ type⟩ indirect through the join type.
    for (size_t i = 0; i < join_type->argc; i++) {
      MuonType *argument = join_type->argv[i];

      Rule *rule = rule_search(inductor, argument, attitude.type);
      // assert(rule != NULL);
      if (rule != NULL)
        rule->center = &join_type->as_type;

      if ((rule = edge_define(inductor, argument, &join_type->as_type)) == NULL)
        abort();
      rule->tag = JOIN_RULE;
      rule->i = i;
    }

    join = &join_type->as_type;
  }

  size_t size = instance_argc;
  if (struct_size_overflow(Semisolution, argv, &size))
    return errno = ENOMEM, NULL;
  if ((solution = malloc(size)) == NULL)
    abort();
  *solution = (Semisolution) {.type = join, .argc = instance_argc};
  assert(solution->type != NULL);

  // size_t j = 0;
  // it = rule_iterator(inductor, attitude);
  // for (const Rule *rule; (rule = rule_next(&it)) != NULL;) {
  //   if (rule->tag == INDIRECT_RULE)
  //     continue;
  //   if (rule->instance == NULL)
  //     continue;
  //
  //   MuonType *nbr_sol = neighbor_solution(
  //       inductor, rule->vertex[charge], charge);
  //   solution->argv[j].instance = rule->instance;
  //   solution->argv[j].type = nbr_sol;
  //   j++;
  // }
  // assert(j == instance_argc);

  return assign_semisolution(inductor, attitude, solution);
}

/// Reduce a type to its solution.
///
/// Performs an iterative depth-first traversal of the type tree using
/// type_next / type_continue / type_return.  On return from each type:
///
///   - Implicit types: compute the attitude-specific solution for the
///     current charge direction via reduce_implicit_attitude.
///
///   - Nonimplicit types: resolve structural implicit children that now
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

    // On return, create the solution of the returned type. If the type is an
    // implicit type, then this is a single charge solution. Otherwise, this is
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

        if (cursor.type != &result->as_type) {
          Rule *rule;
          if ((rule = edge_define(inductor, cursor.type, &result->as_type))
              == NULL)
            return NULL;
        }
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
        struct MuonMeetType *allocation;
        if ((allocation = meet_type_allocate(engine, meet_type->argc)) == NULL)
          return NULL;
        for (size_t i = 0; i < meet_type->argc; i++) {
          MuonType *argument = meet_type->argv[i];
          argument = type_solution(inductor, argument);
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

      case MUON_IMPLICIT_TYPE:
        Semisolution *solution[2] = {};

        if ((solution[cursor.charge] = reduce_implicit_type(inductor, cursor)) == NULL)
          return NULL;

        Attitude invert = attitude_invert(cursor);
        if ((solution[invert.charge] = type_semisolution(inductor, invert)) == NULL)
          break;

        MuonType *result;
        if ((result = implicit_solution(inductor, solution)) == NULL)
          return NULL;

        assign_solution(inductor, cursor.type, result);
        break;

      case MUON_VARIABLE_TYPE:
        assert(cursor.type->explicit);
        assign_solution(inductor, cursor.type, cursor.type);
    }

    cursor = type_return(next = cursor);

    // If we've returned from an implicit type to a nonimplicit type, then check
    // whether the implicit needs its inverted attitude solution. If so,
    // re-enter at the inverted charge. If both attitude solutions already
    // exist, resolve the implicit now.
    if (cursor.type != NULL && is_implicit_type(cursor.type))
      continue;

    if (!is_implicit_type(next.type))
      continue;

    next = attitude_invert(next);
    if (type_semisolution(inductor, next) != NULL)
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

    MuonType *source_type = node_source_type(inductor, node);
    if (reduce_type(inductor, source_type) == NULL)
      goto except;

    MuonType *target_type = node_target_type(inductor, node);
    if (target_type == NULL)
      continue;
    if (reduce_type(inductor, target_type) == NULL)
      goto except;
  } while ((node = node_return(node)) != NULL);

  return node_source_type(inductor, root);

except:
  while ((node = node_return(node)) != NULL) {}
  return NULL;
}
