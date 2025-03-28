#include "../common.h"
#include "core.h"
#include "induce.h"

#include "../stator/node.h"
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
  const mu_type_t *target = coercion->target;

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
 * In this example, origin should be ⟨α ⇒ v⟩ and coercion should be α ⇝ β.
 */
void *redirect_up(
    induce_t *induce, type_edge_t *origin, const mu_coercion_t *coercion) {
  universe_iterator_t it;
  it = universe_iterator(&induce->universe, origin->target, 1);
  for (type_edge_t *c; (c = universe_next(&it)) != NULL;) {
    if (c->target->kind != MU_VARIABLE_TYPE)
      continue;

    type_edge_t *edge = universe_search(&induce->universe, origin->source, c->target);
    assert(edge != NULL);

    const mu_coercion_t *tail;
    if ((tail = coerce_with(edge)) == NULL)
      return NULL;

    const mu_indirect_coercion_t *result;
    if ((result = mu_indirect_coercion(coercion, tail)) == NULL)
      return NULL;
    edge_assign(edge, &result->as_coercion);
  }
  return induce;
}

const mu_solution_t *reduce_type_to_join(
    induce_t *induce, const mu_variable_type_t *variable_type) {
  if (variable_type->join != NULL)
    return &variable_type->join->as_solution;

  universe_t *universe = &induce->universe;
  universe_iterator_t iterator;

  // Reduce each variable type that's a source to this variable type
  iterator = universe_iterator(universe, &variable_type->as_type, 0);
  for (type_edge_t *edge; (edge = universe_next(&iterator)) != NULL;) {
    if (edge->indirect > 1)
      continue;

    // If the edge's source isn't a variable type, then length++
    const mu_variable_type_t *next_variable;
    if ((next_variable = mu_type_cast(edge->source, next_variable)) == NULL)
      continue;

    // Otherwise, reduce it. Then add its join length to length.
    if (reduce_type_to_join(induce, next_variable) == NULL)
      return NULL;
    assert(next_variable->join != NULL);
  }

  // Take each type that we haven't definitely eliminated as a direct source for
  // this variable type (i.e. each edge with indirect = 2), and simplify.
  universe_iterator_t it, jt;
  it = universe_iterator(universe, &variable_type->as_type, 0);
  for (type_edge_t *a; (a = universe_next(&it)) != NULL;) {
    if (a->indirect > 1 || a->source->kind == MU_VARIABLE_TYPE)
      continue;

    const mu_type_t *a_type = a->source;

    jt = universe_iterator(universe, &variable_type->as_type, 0);
    for (type_edge_t *b; (b = universe_next(&jt)) != NULL;) {
      if (a == b)
        continue;
      if (b->indirect > 1 || b->source->kind == MU_VARIABLE_TYPE)
        continue;

      const mu_type_t *b_type = b->source;

      const mu_coercion_t *coercion;
      if ((coercion = retrieve_coercion(induce, b_type, a_type)) == NULL)
        return NULL;
      if (coercion == NO_SUCH_COERCION)
        continue;

      const mu_coercion_t *tail;
      if ((tail = coerce_with(a)) == NULL)
        return NULL;

      const mu_indirect_coercion_t *result;
      if ((result = mu_indirect_coercion(coercion, tail)) == NULL)
        return NULL;
      edge_assign(b, &result->as_coercion);

      if (redirect_up(induce, b, coercion) == NULL)
        return NULL;
    }
  }

  // At this point, each directish edge (indirect <= 1) is an actual type that
  // should be put into the join.

  // Determine the length of the join
  iterator = universe_iterator(universe, &variable_type->as_type, 0);
  size_t length = 0;
  for (type_edge_t *edge; (edge = universe_next(&iterator)) != NULL;) {
    if (edge->indirect > 1 || edge->source->kind == MU_VARIABLE_TYPE)
      continue;
    length += 1;
  }

  // Allocate a join
  mu_join_t *join;
  if ((join = join_allocate(induce, length)) == NULL)
    return NULL;
  join->argc = 0;

  iterator = universe_iterator(universe, &variable_type->as_type, 0);
  for (type_edge_t *edge; (edge = universe_next(&iterator)) != NULL;) {
    if (edge->indirect > 1 || edge->source->kind == MU_VARIABLE_TYPE)
      continue;

    // Handle a normal type
    const mu_join_coercion_t *coercion;
    if ((coercion = mu_join_coercion(variable_type, join->argc)) == NULL)
      return NULL;
    join->argv[join->argc++] = edge->source;

    edge_assign(edge, &coercion->as_coercion);
  }

  const mu_join_t *result;
  if ((result = join_activate(join)) == NULL)
    return NULL;
  ((mu_variable_type_t *) variable_type)->join = result;

  return &result->as_solution;
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
      const mu_type_t *target = edge_coercion->target;

      assert(target->kind == MU_VARIABLE_TYPE || source->kind == MU_VARIABLE_TYPE);

      if (target->kind == MU_VARIABLE_TYPE) {
        const mu_variable_type_t *v = (const mu_variable_type_t *) target;
        if (reduce_type_to_join(induce, v) == NULL)
          return NULL;
      }

      if ((next_coercion = mu_edge_coercion_reload(&induce->universe, edge_coercion)) != NULL)
        return reduce_coercion(induce, next_coercion);

      if (source->kind == MU_VARIABLE_TYPE) {
        const mu_variable_type_t *v = (const mu_variable_type_t *) source;
        if (reduce_type_to_join(induce, v) == NULL)
          return NULL;

        const mu_join_t *source_join = v->join;
        assert(source_join != NULL);

        mu_unjoin_coercion_t *allocation;
        if ((allocation = unjoin_coercion_allocate(source_join->argc)) == NULL)
          return NULL;

        for (size_t i = 0; i < source_join->argc; i++) {
          const type_edge_t *edge;
          edge = universe_search(&induce->universe, source_join->argv[i], target);
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

      __builtin_unreachable();
    }

    case IS_KIND_OF(indirect_coercion): {
      const mu_coercion_t *head = indirect_coercion->head;
      head = reduce_coercion(induce, head);

      const mu_coercion_t *tail = indirect_coercion->tail;
      tail = reduce_coercion(induce, tail);

      const mu_indirect_coercion_t *result;
      if ((result = mu_indirect_coercion(head, tail)) == NULL)
        return NULL;
      return &result->as_coercion;
    }

    case IS_KIND_OF(variance_coercion): {
      const mu_core_type_t *target = variance_coercion->target;

      mu_variance_coercion_t *allocation;
      if ((allocation = variance_coercion_allocate(target)) == NULL)
        return NULL;

      for (size_t i = 0; i < target->core->argc; i++) {
        const mu_coercion_t *argument = variance_coercion->argv[i];
        allocation->argv[i] = reduce_coercion(induce, argument);
      }

      const mu_variance_coercion_t *result;
      if ((result = variance_coercion_activate(allocation)) == NULL)
        return NULL;
      return &result->as_coercion;
    }

    case MU_RECORD_COERCION:
      abort();

    case MU_JOIN_COERCION:
      return coercion;

    case IS_KIND_OF(unjoin_coercion): {
      size_t argc = unjoin_coercion->argc;

      mu_unjoin_coercion_t *allocation;
      if ((allocation = unjoin_coercion_allocate(argc)) == NULL)
        return NULL;

      for (size_t i = 0; i < argc; i++)
        allocation->argv[i] = reduce_coercion(induce, unjoin_coercion->argv[i]);

      const mu_unjoin_coercion_t *result;
      if ((result = unjoin_coercion_activate(allocation, unjoin_coercion->target)) == NULL)
        return NULL;
      return &result->as_coercion;
    }
  }
  __builtin_unreachable();
}

const mu_coercion_t *reduce_coercion_external(
    induce_t *induce, const mu_coercion_t *coercion) {
  debug("Reducing coercion ");
  mu_coercion_debug(coercion);

  const mu_coercion_t *result;
  if ((result = reduce_coercion(induce, coercion)) == NULL)
    return NULL;

  debug(" to ");
  mu_coercion_debug(result);
  debug("\n");

  return result;
}

const mu_type_t *reduce_node(induce_t *induce, const mu_node_t *root) {
  assert(root->id < induce->node_length);

  const mu_node_t *node = root, *next;
  do {
    while ((next = node_at(node, node_cursor(node)->i++)) != NULL)
      node = node_continue(node, next);

    const mu_type_t *source = evince_type(induce, node);
    assert(source != NULL);

    const mu_coercion_t *coercion;
    if ((coercion = induce->node_to_coercion[node->id]) == NULL)
      continue;

    const mu_coercion_t *result;
    if ((result = reduce_coercion_external(induce, coercion)) == NULL)
      return NULL;
    induce->node_to_coercion[node->id] = result;
  } while ((node = node_return(node)) != NULL);

  return evince_type(induce, root);
}
