#include "induce.h"

#include "../stator/node.h"
#include "coercion.h"
#include "type.h"

#include <assert.h>
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

const mu_coercion_t *reduce_coercion(const mu_coercion_t *coercion, const mu_type_t *source) {
  return coercion;
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
    if ((result = reduce_coercion(coercion, source)) == NULL)
      return NULL;
    induce->coercion[node->id] = result;
  } while ((node = node_return(node)) != NULL);

  return evince(induce, root);
}
