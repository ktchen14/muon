#include "../common.h"
#include "core.h"
#include "induce.h"

#include "../engine/node.h"
#include "coercion.h"
#include "type.h"
#include "universe.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

const mu_coercion_t *reduce_coercion(
    induce_t *induce, const mu_coercion_t *coercion);

/// Load the coercion that is assigned to the <em>coercion</em>'s edge. If that
/// coercion is the edge @a coercion itself, then return @c NULL.
__attribute__((nonnull, pure))
const mu_coercion_t *mu_edge_coercion_reload(
    const universe_t *universe, const mu_edge_coercion_t *coercion) {
  const mu_type_t *source = coercion->source;
  const mu_type_t *target = coercion->as_coercion.target;

  type_edge_t *edge = universe_search(universe, source, target);
  assert(edge != NULL);

  const mu_coercion_t *result;
  if ((result = edge->coercion) == &coercion->as_coercion)
    return NULL;
  return result;
}

/**
 * This should be called when we find a triangular type relationship within the
 * source side of a type variable v. If we locate a coercion α ⇝ β, and we have
 * both ⟨α ⇒ v⟩ and ⟨β ⇒ v⟩, then we should make ⟨α ⇒ v⟩ an indirect edge since
 * α is coercible to v through α ⇝ β ⇝ v.
 *
 * Because we maintain the transitive closure of each type variable, we also
 * know that ∃⟨α ⇒ τ⟩ and ∃⟨β ⇒ τ⟩ if ∃⟨v ⇒ τ⟩. Thus, within the source side of
 * each type variable τ that v has an edge to, we should also make ⟨α ⇒ τ⟩ an
 * indirect edge (through α ⇝ β ⇝ τ).
 *
 * In this example, origin should be ⟨α ⇒ v⟩, center should be β, and coercion
 * should be α ⇝ β.
 */
void *redirect_source(
    induce_t *induce,
    type_edge_t *origin,
    const mu_type_t *center,
    const mu_coercion_t *coercion) {
  const mu_type_t *source = origin->source;

  universe_iterator_t it;
  it = universe_iterator(&induce->universe, origin->target, 1);

  // Jump into the loop with τ = v
  type_edge_t *next = origin;
  goto entrance;

  // ∀⟨v ⇒ τ⟩ | τ is a variable type
  for (type_edge_t *direct; (next = universe_next(&it)) != NULL;) {
    if (next->target->kind != MU_VARIABLE_TYPE)
      continue;

    // Locate ⟨α ⇒ τ⟩
    origin = universe_search(&induce->universe, source, next->target);
    assert(origin != NULL);

  entrance:
    // Locate ⟨β ⇒ τ⟩
    direct = universe_search(&induce->universe, center, next->target);
    assert(direct != NULL);

    // Retrieve β ⇝ τ
    const mu_coercion_t *direct_coercion;
    if ((direct_coercion = coerce_with(induce, direct)) == NULL)
      return NULL;

    // Create α ⇝ β ⇝ τ
    const mu_indirect_coercion_t *result;
    if ((result = mu_indirect_coercion(induce, coercion, direct_coercion)) == NULL)
      return NULL;

    // Assign the α ⇝ β ⇝ τ to ⟨α ⇒ τ⟩
    edge_assign(origin, &result->as_coercion);
  }

  return induce;
}

const void *reduce_type_to_join(
    induce_t *induce, const mu_variable_type_t *target) {
  if (target->solution != NULL)
    return target->solution;

  if (target->reduced)
    return target;

  universe_t *universe = &induce->universe;
  universe_iterator_t it;

  // Reduce each variable type that's a source to this variable type
  it = universe_iterator(universe, &target->as_type, 0);
  for (type_edge_t *edge; (edge = universe_next(&it)) != NULL;) {
    if (edge->indirect)
      continue;

    // If the edge's source isn't a variable type, then length++
    const mu_variable_type_t *next_variable;
    if ((next_variable = mu_type_cast(edge->source, next_variable)) == NULL)
      continue;

    // Otherwise, reduce it. Then add its join length to length.
    if (reduce_type_to_join(induce, next_variable) == NULL)
      return NULL;
    /* assert(next_variable->solution != NULL); */
  }

  // For each type pair α and β, where α ≠ β, both are sources to the variable
  // type, and neither is itself a variable type, attempt the coercion α ⇝ β. If
  // no such coercion exists, then attempt the coercion β ⇝ α. If we have either
  // coercion, then make one type indirect.
  //
  // Determine the length of the join to allocate as the number of remaining
  // types that aren't variable types and are sources to the variable type.
  type_edge_t *single_edge;
  const mu_type_t *single_a;
  size_t argc = 0;
  it = universe_iterator(universe, &target->as_type, 0);
  for (type_edge_t *a_edge; (a_edge = universe_next(&it)) != NULL;) {
    const mu_type_t *a;
    if (a_edge->indirect || (a = a_edge->source)->kind == MU_VARIABLE_TYPE)
      continue;

    universe_iterator_t jt = it;
    for (type_edge_t *b_edge; (b_edge = universe_next(&jt)) != NULL;) {
      const mu_type_t *b;
      if (b_edge->indirect || (b = b_edge->source)->kind == MU_VARIABLE_TYPE)
        continue;

      // If we have b ⇝ a, then assign b ⇝ a ⇝ v to ⟨b ⇒ v⟩ and skip this b
      const mu_coercion_t *coercion;
      if ((coercion = retrieve_coercion(induce, b, a)) == NULL)
        return NULL;
      if (coercion != MU_NO_SUCH_COERCION) {
        if (redirect_source(induce, b_edge, a, coercion) == NULL)
          return NULL;
        continue;
      }

      // If we have a ⇝ b, then assign a ⇝ b ⇝ v to ⟨a ⇒ v⟩ and skip this a
      if ((coercion = retrieve_coercion(induce, a, b)) == NULL)
        return NULL;
      if (coercion != MU_NO_SUCH_COERCION) {
        if (redirect_source(induce, a_edge, b, coercion) == NULL)
          return NULL;
        goto continue_a;
      }
    }

    // Record a and a_edge in case we don't need a join and a is the solution
    single_a = a;
    single_edge = a_edge;
    argc++;
  continue_a:;
  }

  if (target->scheme != NULL) {
    it = universe_iterator(universe, &target->as_type, 0);
    for (type_edge_t *edge; (edge = universe_next(&it)) != NULL;) {
      if (edge->coercion != NULL)
        edge->coercion = induce->slot_coercion;
    }

    ((mu_variable_type_t *) target)->reduced = 1;
    return target;
  }

  // If we're left with a single direct edge ⟨α ⇒ target⟩, then we don't have to
  // assign a join type to target at all.
  if (argc == 1) {
    // Assign the id coercion to ⟨α ⇒ target⟩
    edge_assign(single_edge, induce->id_coercion);

    // Assign α ⇝ β to each ⟨target ⇒ β⟩
    const mu_type_t *solution = single_a;
    it = universe_iterator(universe, &target->as_type, 1);
    for (type_edge_t *edge; (edge = universe_next(&it)) != NULL;) {
      type_edge_t *e;
      e = universe_search(&induce->universe, solution, edge->target);
      assert(e != NULL);
      edge->coercion = coerce_with(induce, e);
    }

    // Define ⟨target ⇒ α⟩ and assign the id coercion to it
    type_edge_t *e = edge_define(&induce->universe, &target->as_type, single_a);
    edge_assign(e, induce->id_coercion);

    // Then, assign α as the solution to target and return it.
    return assign_solution(target, solution);
  }

  // Allocate a join
  mu_join_type_t *allocation;
  if ((allocation = join_type_allocate(induce, argc)) == NULL)
    return NULL;
  argc = 0;

  it = universe_iterator(universe, &target->as_type, 0);
  for (type_edge_t *edge; (edge = universe_next(&it)) != NULL;) {
    if (edge->indirect || edge->source->kind == MU_VARIABLE_TYPE)
      continue;

    allocation->argv[argc] = edge->source;

    const mu_join_coercion_t *coercion;
    if ((coercion = mu_join_coercion(induce, &target->as_type, argc++)) == NULL)
      return NULL;
    edge_assign(edge, &coercion->as_coercion);
  }
  assert(argc == allocation->argc);

  const mu_join_type_t *join_type;
  if ((join_type = join_type_activate(allocation)) == NULL)
    return NULL;

  type_edge_t *e;
  e = edge_define(&induce->universe, &target->as_type, &join_type->as_type);
  edge_assign(e, induce->id_coercion);
  e = edge_define(&induce->universe, &join_type->as_type, &target->as_type);
  edge_assign(e, induce->id_coercion);

  return assign_solution(target, &join_type->as_type);
}

const mu_coercion_t *reduce_coercion(
    induce_t *induce, const mu_coercion_t *coercion) {
  switch ON_ABSTRACT_OBJECT(coercion) {
    case MU_ID_COERCION:
      return coercion;

    case IS_KIND_OF(edge_coercion): {
      const mu_coercion_t *next_coercion;
      if ((next_coercion = mu_edge_coercion_reload(&induce->universe, edge_coercion)) != NULL)
        return reduce_coercion(induce, next_coercion);

      const mu_type_t *source = edge_coercion->source;
      const mu_type_t *target = edge_coercion->as_coercion.target;

      assert(target->kind == MU_VARIABLE_TYPE || source->kind == MU_VARIABLE_TYPE);

      if (target->kind == MU_VARIABLE_TYPE) {
        const mu_variable_type_t *v = (const mu_variable_type_t *) target;
        if (reduce_type_to_join(induce, v) == NULL)
          return NULL;

        if (v->scheme != NULL)
          return induce->slot_coercion;
      }

      if ((next_coercion = mu_edge_coercion_reload(&induce->universe, edge_coercion)) != NULL)
        return reduce_coercion(induce, next_coercion);

      if (source->kind == MU_VARIABLE_TYPE) {
        const mu_variable_type_t *v = (const mu_variable_type_t *) source;
        if (reduce_type_to_join(induce, v) == NULL)
          return NULL;
        if (v->scheme != NULL)
          return induce->slot_coercion;
        /* assert(v->solution != NULL); */

        switch ON_ABSTRACT_OBJECT(v->solution) {
          case IS_KIND_OF(core_type): {
            type_edge_t *edge;
            edge = universe_search(&induce->universe, &core_type->as_type, target);
            assert(edge != NULL);

            const mu_coercion_t *coercion = course_coercion(edge);
            assert(coercion != NULL);

            if ((coercion = reduce_coercion(induce, coercion)) == NULL)
              return NULL;
            return edge_assign(edge, coercion);
          }

          case IS_KIND_OF(scheme_type): {
            type_edge_t *edge;
            edge = universe_search(&induce->universe, &scheme_type->as_type, target);
            assert(edge != NULL);

            const mu_coercion_t *coercion = course_coercion(edge);
            assert(coercion != NULL);

            if ((coercion = reduce_coercion(induce, coercion)) == NULL)
              return NULL;
            return edge_assign(edge, coercion);
          }

          case IS_KIND_OF(join_type): {
            mu_unjoin_coercion_t *allocation;
            if ((allocation = unjoin_coercion_allocate(induce, join_type->argc)) == NULL)
              return NULL;

            for (size_t i = 0; i < join_type->argc; i++) {
              const type_edge_t *edge;
              edge = universe_search(&induce->universe, join_type->argv[i], target);
              assert(edge != NULL);

              const mu_coercion_t *coercion = course_coercion(edge);
              assert(coercion != NULL);

              allocation->argv[i] = reduce_coercion(induce, coercion);
            }

            const mu_unjoin_coercion_t *result;
            if ((result = unjoin_coercion_activate(allocation, target)) == NULL)
              return NULL;
            return &result->as_coercion;
          }

          // TODO: this is incorrect
          case MU_VARIABLE_TYPE:
            abort();
        }
      }

      __builtin_unreachable();
    }

    case IS_KIND_OF(indirect_coercion): {
      const mu_coercion_t *head = indirect_coercion->head;
      head = reduce_coercion(induce, head);

      const mu_coercion_t *tail = indirect_coercion->tail;
      tail = reduce_coercion(induce, tail);

      const mu_indirect_coercion_t *result;
      if ((result = mu_indirect_coercion(induce, head, tail)) == NULL)
        return NULL;
      return &result->as_coercion;
    }

    case IS_KIND_OF(instance_coercion):
      return &instance_coercion->as_coercion;

    case IS_KIND_OF(variance_coercion): {
      const mu_core_t *core = variance_coercion->core;

      mu_variance_coercion_t *allocation;
      if ((allocation = variance_coercion_allocate(induce, core)) == NULL)
        return NULL;

      for (size_t i = 0; i < core->argc; i++) {
        const mu_coercion_t *argument = variance_coercion->argv[i];
        allocation->argv[i] = reduce_coercion(induce, argument);
      }

      const mu_variance_coercion_t *result;
      if ((result = variance_coercion_activate(allocation, variance_coercion->as_coercion.target)) == NULL)
        return NULL;
      return &result->as_coercion;
    }

    case MU_SLOT_COERCION:
      return coercion;

    case MU_UNSCHEME_COERCION:
      return coercion;

    case MU_JOIN_COERCION:
      return coercion;

    case IS_KIND_OF(unjoin_coercion): {
      size_t argc = unjoin_coercion->argc;

      mu_unjoin_coercion_t *allocation;
      if ((allocation = unjoin_coercion_allocate(induce, argc)) == NULL)
        return NULL;

      for (size_t i = 0; i < argc; i++)
        allocation->argv[i] = reduce_coercion(induce, unjoin_coercion->argv[i]);

      const mu_unjoin_coercion_t *result;
      if ((result = unjoin_coercion_activate(allocation, unjoin_coercion->as_coercion.target)) == NULL)
        return NULL;
      return &result->as_coercion;
    }

    case IS_KIND_OF(meet_coercion): {
      size_t argc = meet_coercion->argc;

      mu_meet_coercion_t *allocation;
      if ((allocation = meet_coercion_allocate(induce, argc)) == NULL)
        return NULL;

      for (size_t i = 0; i < argc; i++)
        allocation->argv[i] = reduce_coercion(induce, meet_coercion->argv[i]);

      const mu_meet_coercion_t *result;
      if ((result = meet_coercion_activate(allocation, meet_coercion->as_coercion.target)) == NULL)
        return NULL;
      return &result->as_coercion;
    }

    case MU_UNMEET_COERCION:
      return coercion;
  }
  __builtin_unreachable();
}

const mu_type_t *reduce_node(induce_t *induce, MuonNode *root) {
  assert(root->id < induce->node_length);

  MuonNode *node = root, *next;
  do {
    while ((next = node_at(node, node_cursor(node)->i++)) != NULL)
      node = node_continue(node, next);

    const mu_coercion_t *coercion;
    const mu_type_t *target_type;
    if ((coercion = evince_coercion(induce, node, &target_type)) == NULL)
      continue;

    const mu_coercion_t *result;
    if ((result = reduce_coercion(induce, coercion)) == NULL)
      return NULL;
    override_coercion(induce, node, result, target_type);
  } while ((node = node_return(node)) != NULL);

  return evince_type(induce, root);
}
