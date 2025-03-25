#include "../common.h"
#include "core.h"
#include "induce.h"

#include "../stator/node.h"
#include "coercion.h"
#include "type.h"
#include "universe.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define evince induce_reveal

/* const mu_type_t *reduce_type(induce_t *induce, const mu_type_t *type, _Bool negative); */

/* const mu_coercion_t *make_coercion( */
/*     induce_t *induce, const mu_type_t *source, const mu_type_t *target) { */
/*   assert(source->kind != MU_SCHEME_TYPE); */
/*   assert(target->kind != MU_SCHEME_TYPE); */

/*   if (source == target) */
/*     return &induce->id_coercion->as_coercion; */

/*   fprintf(stderr, "Making coercion from "); */
/*   type_debug(source, 0); */
/*   fprintf(stderr, " to "); */
/*   type_debug(target, 0); */
/*   fprintf(stderr, "\n"); */

/*   const induce_edge_t *edge = universe_search(&induce->universe, source, target); */
/*   assert(edge != NULL); */
/*   assert(edge->tactic != NULL); */

/*   // If the source type is a join type, then return an unjoin coercion with a */
/*   // coercion for each discriminant in the join type */
/*   if (edge->tactic->kind == UNJOIN_TACTIC) { */
/*     const unjoin_tactic_t *tactic = (const unjoin_tactic_t *) edge->tactic; */

/*     mu_unjoin_coercion_t *allocation; */
/*     if ((allocation = unjoin_coercion_allocate(tactic->length)) == NULL) */
/*       return NULL; */

/*     for (size_t i = 0; i < induce->universe.length; i++) { */
/*       const induce_edge_t *join_edge = &induce->universe.data[i]; */
/*       if (join_edge->target != source) */
/*         continue; */
/*       assert(join_edge->tactic != NULL); */
/*       assert(join_edge->tactic->kind == JOIN_TACTIC); */

/*       const join_tactic_t *join_tactic = (const join_tactic_t *) join_edge->tactic; */

/*       const mu_coercion_t *coercion; */
/*       if ((coercion = make_coercion(induce, join_edge->source, target)) == NULL) */
/*         return NULL; */
/*       allocation->argv[join_tactic->i] = coercion; */
/*     } */

/*     const mu_unjoin_coercion_t *result; */
/*     if ((result = unjoin_coercion_activate(allocation)) == NULL) */
/*       return NULL; */
/*     ((mu_coercion_t *) result)->target = target; */
/*     return &result->as_coercion; */
/*   } */

/*   // If the target type is a join type */
/*   if (edge->tactic->kind == JOIN_TACTIC) { */
/*     join_tactic_t *tactic = (join_tactic_t *) edge->tactic; */

/*     const mu_join_coercion_t *result; */
/*     if ((result = mu_join_coercion(tactic->i)) == NULL) */
/*       return NULL; */
/*     ((mu_coercion_t *) result)->target = target; */
/*     return &result->as_coercion; */
/*   } */

/*   const tactic_t *tactic = edge->tactic; */
/*   if (tactic == NULL) */
/*     return &induce->id_coercion->as_coercion; */

/*   if (tactic->kind == RECORD_TACTIC) { */
/*     const record_tactic_t *record_tactic = (const record_tactic_t *) tactic; */
/*     const record_instance_t *instance = record_tactic->instance; */

/*     assert(source->kind == MU_CORE_TYPE && target->kind == MU_CORE_TYPE); */
/*     const mu_core_type_t *source_core_type = (const mu_core_type_t *) source; */
/*     const mu_core_type_t *target_core_type = (const mu_core_type_t *) target; */

/*     mu_record_coercion_t *allocation; */
/*     if ((allocation = record_coercion_allocate(instance)) == NULL) */
/*       return NULL; */

/*     for (size_t i = 0; i < instance->target->argc; i++) { */
/*       const mu_type_t *next_target = target_core_type->argv[i]; */
/*       const mu_type_t *next_source = source_core_type->argv[instance->argv[i]]; */

/*       const mu_coercion_t *coercion; */
/*       if ((coercion = make_coercion(induce, next_source, next_target)) == NULL) */
/*         return NULL; */
/*       allocation->argv[i] = coercion; */
/*     } */

/*     const mu_record_coercion_t *result; */
/*     if ((result = record_coercion_activate(allocation)) == NULL) */
/*       return NULL; */

/*     ((mu_coercion_t *) result)->target = target; */
/*     return &result->as_coercion; */
/*   } */

/*   assert(source->kind == MU_CORE_TYPE && target->kind == MU_CORE_TYPE); */
/*   const mu_core_type_t *source_core_type = (const mu_core_type_t *) source; */
/*   const mu_core_t *source_core = source_core_type->core; */
/*   const mu_core_type_t *target_core_type = (const mu_core_type_t *) target; */
/*   const mu_core_t *target_core = target_core_type->core; */

/*   assert(source_core == target_core); */

/*   const mu_core_t *core = source_core; */

/*   mu_variance_coercion_t *allocation; */
/*   if ((allocation = variance_coercion_allocate(core)) == NULL) */
/*     return NULL; */

/*   for (size_t i = 0; i < core->argc; i++) { */
/*     const mu_type_t *next_source = source_core_type->argv[i]; */
/*     const mu_type_t *next_target = target_core_type->argv[i]; */

/*     assert(core->argv[i].variance != MU_INVARIANCE); */
/*     if (core->argv[i].variance == MU_CONTRAVARIANCE) { */
/*       const mu_type_t *t = next_source; next_source = next_target; next_target = t; */
/*     } */
/*     allocation->argv[i] = make_coercion(induce, next_source, next_target); */
/*   } */

/*   const mu_variance_coercion_t *result; */
/*   if ((result = variance_coercion_activate(allocation)) == NULL) */
/*     return NULL; */
/*   ((mu_coercion_t *) result)->target = target; */
/*   return &result->as_coercion; */
/* } */

/* const mu_type_t *reduce_type(induce_t *induce, const mu_type_t *type, _Bool negative) { */
/*   assert(type->kind != MU_SCHEME_TYPE); */

/*   const mu_core_type_t *core_type; */
/*   if ((core_type = mu_type_cast(type, core_type)) != NULL) { */
/*     const mu_core_t *core = core_type->core; */

/*     for (size_t i = 0; i < core->argc; i++) { */
/*       const mu_type_t *argument = core_type->argv[i]; */

/*       _Bool argument_negative = negative; */
/*       mu_variance_t variance = core->argv[i].variance; */
/*       assert(variance != MU_INVARIANCE); */
/*       if (variance == MU_CONTRAVARIANCE) */
/*         argument_negative = !argument_negative; */

/*       const mu_type_t *assignment; */
/*       if ((assignment = reduce_type(induce, argument, argument_negative)) == NULL) */
/*         return NULL; */
/*     } */

/*     return &core_type->as_type; */
/*   } */

/*   const mu_variable_type_t *variable_type; */
/*   if ((variable_type = mu_type_cast(type, variable_type)) != NULL) { */
/*     /1* if (!negative) { *1/ */
/*       // Reduce each subtype of the variable type */
/*       size_t j = 0; */
/*       for (size_t i = 0; i < induce->universe.length; i++) { */
/*         induce_edge_t *edge = &induce->universe.data[i]; */
/*         if (edge->target == &variable_type->as_type) { */
/*           if (reduce_type(induce, edge->source, negative) == NULL) */
/*             return NULL; */
/*           edge->tactic = &join_tactic_create(j++)->as_tactic; */
/*         } */
/*       } */

/*       for (size_t i = 0; i < induce->universe.length; i++) { */
/*         induce_edge_t *edge = &induce->universe.data[i]; */
/*         if (edge->source == &variable_type->as_type) { */
/*           edge->tactic = &unjoin_tactic_create(j)->as_tactic; */
/*         } */
/*       } */

/*       return &variable_type->as_type; */
/*     /1* } else { *1/ */
/*     /1*   assert(!"Unimplemented reduction of negative variable"); *1/ */
/*     /1* } *1/ */
/*   } */

/*   assert(0); */
/* } */

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

const mu_variable_type_t *reduce_type_to_join(
    induce_t *induce, const mu_variable_type_t *variable_type) {
  if (variable_type->join != NULL)
    return variable_type;

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
  size_t size;
  if (rare((size = struct_size(mu_join_t, argv, length)) == 0))
    return errno = ENOMEM, NULL;

  mu_join_t *join;
  if ((join = malloc(size)) == NULL)
    return NULL;
  join->argc = length;

  iterator = universe_iterator(universe, &variable_type->as_type, 0);
  size_t join_i = 0;
  for (edge_t *edge; (edge = universe_next(&iterator)) != NULL;) {
    if (edge->indirect)
      continue;

    // Handle a normal type
    const mu_variable_type_t *next_variable;
    if ((next_variable = mu_type_cast(edge->source, next_variable)) == NULL) {
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
  }

  ((mu_variable_type_t *) variable_type)->join = join;
  return variable_type;
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

      if (target->kind == MU_VARIABLE_TYPE) {
        const mu_variable_type_t *v = (const mu_variable_type_t *) target;
        if (reduce_type_to_join(induce, v) == NULL)
          return NULL;

        next_coercion = mu_edge_coercion_load(&induce->universe, edge_coercion);
        assert(next_coercion != NULL);
        return reduce_coercion(induce, next_coercion);
      } else if (source->kind == MU_VARIABLE_TYPE) {
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

      abort();

      // TODO
      return coercion;
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
