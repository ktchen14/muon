#include "reduce.h"

#include "common.h"
#include "induce.h"
#include "node.h"

#include <assert.h>
#include <errno.h>
#include <stdckdint.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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
  assert(semisolution[0] != NULL && semisolution[1] != NULL);

  if (is_bottom_type(semisolution[0]->type))
    return semisolution[0]->type;

  // if (is_object_type(semisolution[1]->type))
  //   return semisolution[1]->type;

  if (semisolution[0]->type->tag != MUON_JOIN_TYPE)
    return semisolution[0]->type;

  // if (semisolution[1]->type->tag != MUON_MEET_TYPE)
  //   return semisolution[1]->type;

  return semisolution[0]->type;
}

int type_cmp(const void *a, const void *b) {
  MuonType *ra = *(MuonType *const *) a;
  MuonType *rb = *(MuonType *const *) b;

  if ((uintptr_t) ra < (uintptr_t) rb)
    return -1;
  if ((uintptr_t) ra > (uintptr_t) rb)
    return +1;
  return 0;
}

static Vector(MuonType *) simplify(
    Inductor *inductor, Vector(MuonType *) vector) {
  if (vector_length(vector) < 2)
    return vector;

  // Deduplicate if we have more than a single origin type
  if (vector_length(vector) > 1) {
    qsort(vector, vector_length(vector), sizeof(MuonType *), type_cmp);

    size_t j = 0;
    for (size_t i = 1; i < vector_length(vector); i++) {
      if (vector[i] != vector[j])
        vector[++j] = vector[i];
    }

    vector_length(vector) = ++j;
  }

  size_t origin_length = vector_length(vector);
  for (size_t i = 0; i < origin_length; i++) {
    size_t new_start = vector_length(vector);

    // Append the next origin type to the vector. If it's a join or meet type,
    // expand it (recursively).
    MuonType *origin_type = vector[i];
    if ((vector = vector_append(vector, &origin_type)) == NULL)
      return NULL;

    for (size_t j = new_start; j < vector_length(vector);) {
      MuonJoinType *join_type;
      if ((join_type = muon_type_cast(vector[j], join_type)) == NULL) {
        j++;
        continue;
      }

      if (join_type->argc == 0) {
        vector[j] = vector[--vector_length(vector)];
        continue;
      }

      vector[j] = join_type->argv[0];
      MuonType *const *argv = &join_type->argv[1];
      size_t argc = join_type->argc - 1;
      if ((vector = vector_extend(vector, argv, argc)) == NULL)
        return NULL;
    }

    size_t extant_end = new_start;

    // Filter: check each new proposed type against the current list.
    // Is the proposed type coercible to any current type? If so, discard it.
    for (size_t k = origin_length; k < extant_end; k++) {
      MuonType *extant = vector[k];

      for (size_t j = new_start; j < vector_length(vector);) {
        MuonType *type = vector[j];

        Rule *rule;
        if ((rule = type_assess(inductor, type, extant)) == NULL)
          return NULL;
        if (rule->tag == REJECTED_RULE) {
          j++;
          continue;
        }

        vector[j] = vector[--vector_length(vector)];
      }
    }

    // Is any current type coercible to the proposed type? If so, replace it
    // and remove any other current types also coercible to it.
    for (size_t j = new_start; j < vector_length(vector); j++) {
      MuonType *type = vector[j];

      for (size_t k = origin_length; k < extant_end;) {
        MuonType *extant = vector[k];

        Rule *rule;
        if ((rule = type_assess(inductor, extant, type)) == NULL)
          return NULL;
        if (rule->tag == REJECTED_RULE) {
          k++;
          continue;
        }

        vector[k] = vector[--extant_end];
      }
    }

    size_t size = sizeof(MuonType *) * (vector_length(vector) - new_start);
    memmove(&vector[extant_end], &vector[new_start], size);
    vector_length(vector) -= new_start - extant_end;
  }

  // Move the current list to the front and truncate.
  size_t length = vector_length(vector) - origin_length;
  memmove(vector, vector + origin_length, sizeof(MuonType *[length]));
  vector_truncate(vector, length);

  return vector;
}

/// Reduce an implicit type to a semisolution
static Semisolution *reduce_implicit_type(
    Inductor *inductor, Attitude attitude) {
  assert(is_implicit_type(attitude.type));

  Semisolution *solution;
  if ((solution = type_semisolution(inductor, attitude)) != NULL)
    return solution;

  vector_length(inductor->vector) = 0;

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

    MuonType *type = semisolution->type;
    Vector(MuonType *) vector;
    if ((vector = vector_append(inductor->vector, &type)) == NULL)
      return NULL;
    inductor->vector = vector;
  }

  Vector(MuonType *) vector;
  if ((vector = simplify(inductor, inductor->vector)) == NULL)
    return NULL;
  inductor->vector = vector;

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
      return NULL;
    join = &join_type->as_type;

    // ∀(τ, ...) in J = Join(τ, ...), create ⟨τ ⇒ J⟩; then make ⟨τ ⇒ type⟩
    // indirect through J.
    for (size_t i = 0; i < join_type->argc; i++) {
      MuonType *argument = join_type->argv[i];

      Rule *rule;
      if ((rule = rule_search(inductor, argument, join)) == NULL) {
        if ((rule = edge_define(inductor, argument, join)) == NULL)
          return NULL;
        rule->tag = JOIN_RULE;
        rule->i = i;
      }

      rule = rule_search(inductor, argument, attitude.type);
      // assert(rule != NULL);
      if (rule != NULL)
        rule->center = &join_type->as_type;
    }
  }

  size_t size = instance_argc;
  if (struct_size_overflow(Semisolution, argv, &size))
    return errno = ENOMEM, NULL;
  if ((solution = malloc(size)) == NULL)
    abort();
  *solution = (Semisolution) {.type = join, .argc = 0};
  assert(solution->type != NULL);

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
        _Bool charge = cursor.charge;

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
