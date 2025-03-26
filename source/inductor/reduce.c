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

#define evince induce_reveal

/// Load the coercion that is assigned to the <em>coercion</em>'s edge. If that
/// coercion is the edge @a coercion itself, then return @c NULL.
__attribute__((nonnull, pure))
const mu_coercion_t *mu_edge_coercion_load(
    const universe_t *universe, const mu_edge_coercion_t *coercion) {
  const mu_type_t *source = coercion->source;
  const mu_type_t *target = coercion->as_coercion.target;

  universe_edge_t *edge = universe_search(universe, source, target);
  assert(edge != NULL);

  const mu_coercion_t *next_coercion;
  if ((next_coercion = edge->coercion) == &coercion->as_coercion)
    return NULL;
  return next_coercion;
}

const mu_solution_t *reduce_type_to_join(
    induce_t *induce, const mu_variable_type_t *variable_type) {
  if (variable_type->join != NULL)
    return &variable_type->join->as_solution;

  universe_t *universe = &induce->universe;
  universe_iterator_t iterator;
  typedef universe_edge_t edge_t;

  // Determine the length of the join
  iterator = universe_iterator(universe, &variable_type->as_type, 0);
  size_t length = 0;
  for (edge_t *edge; (edge = universe_next(&iterator)) != NULL;) {
    if (edge->indirect)
      continue;

    // Ensure that the edge has a "normal" coercion. Either no coercion, or an
    // edge coercion that maps back to itself.
    const mu_coercion_t *coercion = edge->coercion;
    if (coercion != NULL) {
      const mu_edge_coercion_t *edge_coercion;
      edge_coercion = mu_coercion_cast(coercion, edge_coercion);
      assert(edge_coercion != NULL);
      assert(edge_coercion->source == edge->source);
      assert(edge_coercion->as_coercion.target == edge->target);
    }

    // If the edge's source isn't a variable type, then length++
    const mu_variable_type_t *next_variable;
    if ((next_variable = mu_type_cast(edge->source, next_variable)) == NULL) {
      length++;
      continue;
    }

    // Otherwise, reduce it. Then add its join length to length.
    if (reduce_type_to_join(induce, next_variable) == NULL)
      return NULL;
    assert(next_variable->join != NULL);
    length += next_variable->join->argc;
  }

  // Allocate a join
  mu_join_t *join;
  if ((join = join_allocate(induce, length)) == NULL)
    return NULL;

  iterator = universe_iterator(universe, &variable_type->as_type, 0);
  size_t join_i = 0;
  for (edge_t *edge; (edge = universe_next(&iterator)) != NULL;) {
    if (edge->indirect)
      continue;

    const mu_type_t *source = edge->source;

    // Handle a normal type
    const mu_variable_type_t *next_variable;
    if ((next_variable = mu_type_cast(source, next_variable)) == NULL) {
      for (size_t i = 0; i < join_i; i++) {
        const mu_coercion_t *coercion;
        if ((coercion = retrieve_coercion(induce, source, join->argv[i])) == NULL)
          return NULL;
        if (coercion == NO_SUCH_COERCION)
          continue;

        const mu_join_coercion_t *join_coercion;
        if ((join_coercion = mu_join_coercion(i)) == NULL)
          return NULL;

        const mu_indirect_coercion_t *result;
        if ((result = mu_indirect_coercion(coercion, &join_coercion->as_coercion)) == NULL)
          return NULL;
        edge->coercion = &result->as_coercion;
        goto next_source;
      }

      const mu_join_coercion_t *join_coercion;
      if ((join_coercion = mu_join_coercion(join_i)) == NULL)
        return NULL;
      edge->coercion = &join_coercion->as_coercion;
      join->argv[join_i++] = edge->source;
      continue;
    }

    // Okay. We have another variable type. We have make an unjoin coercion into
    // a join coercion.
    const mu_join_t *source_join = next_variable->join;

    mu_unjoin_coercion_t *allocation;
    if ((allocation = unjoin_coercion_allocate(source_join->argc)) == NULL)
      return NULL;

    for (size_t i = 0; i < source_join->argc; i++) {
      const mu_join_coercion_t *join_coercion;
      if ((join_coercion = mu_join_coercion(join_i)) == NULL)
        return NULL;
      allocation->argv[i] = &join_coercion->as_coercion;
      join->argv[join_i++] = source_join->argv[i];
    }

    const mu_unjoin_coercion_t *unjoin_coercion;
    if ((unjoin_coercion = unjoin_coercion_activate(allocation)) == NULL)
      return NULL;
    edge->coercion = &unjoin_coercion->as_coercion;

  next_source:;
  }
  join->argc = join_i;

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
      if ((next_coercion = mu_edge_coercion_load(&induce->universe, edge_coercion)) != NULL)
        return reduce_coercion(induce, next_coercion);

      const mu_type_t *source = edge_coercion->source;
      const mu_type_t *target = edge_coercion->as_coercion.target;

      assert(target->kind == MU_VARIABLE_TYPE || source->kind == MU_VARIABLE_TYPE);

      if (target->kind == MU_VARIABLE_TYPE) {
        const mu_variable_type_t *v = (const mu_variable_type_t *) target;
        if (reduce_type_to_join(induce, v) == NULL)
          return NULL;

        next_coercion = mu_edge_coercion_load(&induce->universe, edge_coercion);
        assert(next_coercion != NULL);
        return reduce_coercion(induce, next_coercion);
      } else {
        const mu_variable_type_t *v = (const mu_variable_type_t *) source;
        if (reduce_type_to_join(induce, v) == NULL)
          return NULL;

        const mu_join_t *source_join = v->join;
        assert(source_join != NULL);

        mu_unjoin_coercion_t *allocation;
        if ((allocation = unjoin_coercion_allocate(source_join->argc)) == NULL)
          return NULL;

        for (size_t i = 0; i < source_join->argc; i++) {
          const universe_edge_t *item_edge;
          item_edge = universe_search(&induce->universe, source_join->argv[i], target);
          assert(item_edge != NULL);

          const mu_coercion_t *item_coercion = item_edge->coercion;
          assert(item_coercion != NULL);

          allocation->argv[i] = reduce_coercion(induce, item_coercion);
        }

        const mu_unjoin_coercion_t *result;
        if ((result = unjoin_coercion_activate(allocation)) == NULL)
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
      const mu_core_t *core = variance_coercion->core;

      mu_variance_coercion_t *allocation;
      if ((allocation = variance_coercion_allocate(core)) == NULL)
        return NULL;

      for (size_t i = 0; i < core->argc; i++)
        allocation->argv[i] = reduce_coercion(induce, variance_coercion->argv[i]);

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
      if ((result = unjoin_coercion_activate(allocation)) == NULL)
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

    const mu_coercion_t *coercion;
    if ((coercion = induce->coercion[node->id]) == NULL)
      continue;

    const mu_type_t *source = evince(induce, node);
    assert(source != NULL);

    const mu_coercion_t *result;
    if ((result = reduce_coercion_external(induce, coercion)) == NULL)
      return NULL;
    induce->coercion[node->id] = result;
  } while ((node = node_return(node)) != NULL);

  return evince(induce, root);
}
