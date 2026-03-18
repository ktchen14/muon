#include "reduce.h"

#include "common.h"
#include "induce.h"
#include "node.h"

#include <assert.h>
#include <errno.h>
#include <stdckdint.h>
#include <stdio.h>
#include <stdlib.h>

static inline Attitude type_next(const Inductor *inductor, Attitude origin) {
  if (origin.type->tag != MUON_VARIABLE_TYPE)
    return type_at(origin, type_cursor(origin)->i++);

  size_t i;
  while ((i = type_cursor(origin)->i++) < inductor->rule_length) {
    const Rule *edge = &inductor->edge[i];

    Attitude vertex = attitude_decode(edge->vertex[!origin.charge]);
    if (attitude_eq(vertex, origin))
      return attitude_decode(edge->vertex[origin.charge]);
  }

  return (Attitude) {};
}

static inline MuonType *assign_solution(
    Inductor *inductor, MuonType *type, MuonType *solution) {
  assert(type->id < inductor->type_length);
  return inductor->solution[type->id] = solution;
}

MuonType *reduce_type(Inductor *inductor, Attitude attitude);

MuonType *reduce_type_to_join(Inductor *inductor, MuonVariableType *target) {
  MuonType *solution;
  if ((solution = type_solution(inductor, &target->as_type)) != NULL)
    return solution;

  size_t argc = 0;

  RuleIterator it;

  // Reduce each type that's a source to this type
  it = rule_iterator(inductor, (Attitude) {&target->as_type, 0});
  for (Rule *rule; (rule = rule_next(&it)) != NULL;) {
    if (rule->tag == INDIRECT_RULE)
      continue;

    // Otherwise, reduce it. Then add its join length to length.
    MuonType *solution;
    if ((solution = reduce_type(
             inductor, (Attitude) {attitude_decode(rule->source).type, 0}))
        == NULL)
      return NULL;

    MuonJoinType *join_type;
    if ((join_type = muon_type_cast(solution, join_type)) != NULL) {
      if (rare(ckd_add(&argc, argc, join_type->argc)))
        return errno = ENOMEM, NULL;
    } else {
      if (rare(ckd_add(&argc, argc, 1)))
        return errno = ENOMEM, NULL;
    }
    /* assert(next_variable->solution != NULL); */
  }

  // For each type pair α and β, where α ≠ β, both are sources to the variable
  // type, and neither is itself a variable type, attempt the coercion α ⇝ β. If
  // no such coercion exists, then attempt the coercion β ⇝ α. If we have either
  // coercion, then make one type indirect.
  //
  // Determine the length of the join to allocate as the number of remaining
  // types that aren't variable types and are sources to the variable type.
  Rule *single_edge;
  MuonType *single_a;
  argc = 0;
  it = rule_iterator(inductor, (Attitude) {&target->as_type, 0});
  for (Rule *a_edge; (a_edge = rule_next(&it)) != NULL;) {
    if (a_edge->tag == INDIRECT_RULE)
      continue;
    MuonType *a = type_solution(inductor, attitude_decode(a_edge->source).type);
    assert(a != NULL);

    RuleIterator jt = it;
    for (Rule *b_edge; (b_edge = rule_next(&jt)) != NULL;) {
      if (b_edge->tag == INDIRECT_RULE)
        continue;
      MuonType *b = type_solution(
          inductor, attitude_decode(b_edge->source).type);
      assert(b != NULL);

      // If we have b ⇝ a, then assign b ⇝ a ⇝ v to ⟨b ⇒ v⟩ and skip this b
      const Rule *b_to_a;
      if ((b_to_a = type_assess(inductor, b, a)) == NULL)
        return NULL;
      if (b_to_a->tag != IMPOSSIBLE_RULE) {
        b_edge->tag = INDIRECT_RULE;
        b_edge->center = a;
        continue;
      }

      // If we have a ⇝ b, then assign a ⇝ b ⇝ v to ⟨a ⇒ v⟩ and skip this a
      const Rule *a_to_b;
      if ((a_to_b = type_assess(inductor, a, b)) == NULL)
        return NULL;
      if (a_to_b->tag != IMPOSSIBLE_RULE) {
        a_edge->tag = INDIRECT_RULE;
        a_edge->center = b;
        goto continue_a;
      }
    }

    // Record a and a_edge in case we don't need a join and a is the solution
    single_a = a;
    single_edge = a_edge;
    argc++;
  continue_a:;
  }

  // If we're left with a single direct edge ⟨α ⇒ target⟩, then we don't have to
  // assign a join type to target at all.
  if (argc == 1) {
    // Assign the id coercion to ⟨α ⇒ target⟩
    single_edge->tag = INDIRECT_RULE;
    single_edge->center = &target->as_type;

    // Mark each ⟨target ⇒ β⟩ as indirect through solution
    MuonType *solution = single_a;
    // single_edge->tag = INDIRECT_RULE;
    // single_edge->center = solution;
    // it = rule_iterator(inductor, &target->as_type, 1);
    // for (Rule *edge; (edge = rule_next(&it)) != NULL;) {
    //   edge->tag = INDIRECT_RULE;
    //   edge->center = solution;
    // }

    // Define ⟨target ⇒ α⟩ as an identity rule
    Rule *e = edge_define(inductor, &target->as_type, single_a);
    e->tag = ID_RULE;

    // Then, assign α as the solution to target and return it.
    return assign_solution(inductor, &target->as_type, solution);
  }

  // Allocate a join
  struct MuonJoinType *allocation;
  if ((allocation = join_type_allocate(inductor->engine, argc)) == NULL)
    return NULL;
  argc = 0;

  it = rule_iterator(inductor, (Attitude) {&target->as_type, 0});
  for (Rule *edge; (edge = rule_next(&it)) != NULL;) {
    if (edge->tag == INDIRECT_RULE)
      continue;

    MuonType *source = type_solution(
        inductor, attitude_decode(edge->source).type);
    assert(source != NULL);

    allocation->argv[argc] = source;

    Rule *rule;
    if ((rule = edge_define(inductor, source, &allocation->as_type)) == NULL)
      return NULL;
    rule->tag = JOIN_RULE;
    rule->i = argc++;

    // edge->tag = INDIRECT_RULE;
    // edge->center = &allocation->as_type;
  }
  assert(argc == allocation->argc);

  MuonJoinType *join_type;
  if ((join_type = join_type_activate(allocation)) == NULL)
    return NULL;

  Rule *e;
  e = edge_define(inductor, &target->as_type, &join_type->as_type);
  e->tag = ID_RULE;
  // e = edge_define(inductor, &join_type->as_type, &target->as_type);
  // e->tag = ID_RULE;

  return assign_solution(inductor, &target->as_type, &join_type->as_type);
}

MuonType *reduce_type(Inductor *inductor, Attitude attitude) {
  MuonType *solution;
  if ((solution = type_solution(inductor, attitude.type)) != NULL)
    return solution;

  switch ON_ABSTRACT_TYPE(attitude.type) {
    case IS_CONCRETE_TYPE(MuonCoreType *core_type) {
      MuonCore *core = core_type->core;

      struct MuonCoreType *allocation;
      if ((allocation = core_type_allocate(inductor->engine, core)) == NULL)
        return NULL;

      for (size_t i = 0; i < core_argc(core); i++) {
        MuonCoreMember member = core_at(core, i);

        MuonType *argument = core_type->argv[member.i];
        _Bool charge = attitude.charge ^ member.variance;
        Attitude next = {argument, charge};

        if ((argument = reduce_type(inductor, next)) == NULL)
          return NULL;
        allocation->argv[member.i] = argument;
      }

      MuonCoreType *result;
      if ((result = core_type_activate(allocation)) == NULL)
        return NULL;
      solution = &result->as_type;
      break;
    }

    case IS_CONCRETE_TYPE(MuonVariableType *variable_type) {
      solution = reduce_type_to_join(inductor, variable_type);
      break;
    }

    case MUON_JOIN_TYPE:
      solution = attitude.type;
      break;

    case MUON_MEET_TYPE:
      abort();

    case IS_CONCRETE_TYPE(MuonSchemeType *scheme_type) {
      Attitude next = {scheme_type->matter, attitude.charge};
      if ((solution = reduce_type(inductor, next)) == NULL)
        return NULL;
      break;
    }
  }

  return assign_solution(inductor, attitude.type, solution);
}

MuonType *reduce_node(Inductor *inductor, MuonNode *root) {
  // assert(root->id < inductor->node_number);

  MuonNode *node = root;
  do {
    MuonNode *next;
    while ((next = node_at(node, node_cursor(node)->i++)) != NULL)
      node = node_continue(node, next);

    MuonType *source_type = node_source_type(inductor, node);
    reduce_type(inductor, (Attitude) {source_type, 0});

    MuonType *solution = type_solution(inductor, source_type);
    assign_solution(inductor, source_type, solution);

    // debug("Solving ");
    // muon_type_debug(source_type);
    // debug(" to ");
    // if (solution != NULL)
    //   muon_type_debug(solution);
    // else
    //   debug("NULL");
    // debug("\n");
  } while ((node = node_return(node)) != NULL);

  return node_source_type(inductor, root);
}
