#include "induce.h"
#include "universe.h"

#include "detect.h"
#include "../stator.h"
#include "../status.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

_Thread_local induce_t *debug_induce;

static inline const mu_coercion_t *ensure_core_coercion(
    induce_t *induce, const mu_core_type_t *source, const mu_core_type_t *target);

static inline const mu_coercion_t *ensure_cv(
    induce_t *induce, const mu_type_t *source, const mu_type_t *target);

static inline const mu_coercion_t *retrieve_core_coercion(
    induce_t *induce, const mu_core_type_t *source, const mu_core_type_t *target) {
  const mu_core_t *source_core = source->core;
  const mu_core_t *target_core = target->core;

  if (source_core != target_core)
    return NO_SUCH_COERCION;

  const mu_core_t *core = source_core;

  mu_variance_coercion_t *allocation;
  if ((allocation = variance_coercion_allocate(core)) == NULL)
    return NULL;

  for (size_t i = 0; i < core->argc; i++) {
    const mu_type_t *next_source = source->argv[i];
    const mu_type_t *next_target = target->argv[i];

    mu_variance_t variance = core->argv[i].variance;
    assert(variance != MU_INVARIANCE);
    if (variance == MU_CONTRAVARIANCE) {
      const mu_type_t *t = next_source; next_source = next_target; next_target = t;
    }

    const mu_coercion_t *coercion;
    if ((coercion = retrieve_coercion(induce, next_source, next_target)) == NULL)
      return NULL;
    if (coercion == NO_SUCH_COERCION) {
      free(allocation);
      return coercion;
    }
    allocation->argv[i] = coercion;
  }

  const mu_variance_coercion_t *result;
  if ((result = variance_coercion_activate(allocation)) == NULL)
    return NULL;
  return &result->as_coercion;
}

const mu_coercion_t *retrieve_coercion(
    induce_t *induce, const mu_type_t *source, const mu_type_t *target) {
  assert(source->kind != MU_SCHEME_TYPE && target->kind != MU_SCHEME_TYPE);

  if (source == target)
    return induce->id_coercion;

  // If we already have an edge source => target then
  const universe_edge_t *edge;
  if ((edge = universe_search(&induce->universe, source, target)) != NULL) {
    // If the edge has a coercion, return it
    if (edge->coercion != NULL)
      return edge->coercion;

    // Otherwise, return an edge coercion for the edge
    const mu_edge_coercion_t *result;
    if ((result = mu_edge_coercion(source, target)) == NULL)
      return NULL;
    return &result->as_coercion;
  }

  if (source->kind == MU_CORE_TYPE && target->kind == MU_CORE_TYPE) {
    const mu_core_type_t *next_source = (const mu_core_type_t *) source;
    const mu_core_type_t *next_target = (const mu_core_type_t *) target;

    const mu_coercion_t *result;
    if ((result = retrieve_core_coercion(induce, next_source, next_target)) == NULL)
      return NULL;
    return result;
  }

  return NO_SUCH_COERCION;
}

const mu_coercion_t *ensure_coercion(
    induce_t *induce, const mu_type_t *source, const mu_type_t *target) {
  assert(source->kind != MU_SCHEME_TYPE && target->kind != MU_SCHEME_TYPE);

  if (source == target)
    return induce->id_coercion;

  // If we already have an edge source => target then
  const universe_edge_t *edge;
  if ((edge = universe_search(&induce->universe, source, target)) != NULL) {
    // If the edge has a coercion, return it
    if (edge->coercion != NULL)
      return edge->coercion;

    // Otherwise, return an edge coercion for the edge
    const mu_edge_coercion_t *result;
    if ((result = mu_edge_coercion(source, target)) == NULL)
      return NULL;
    return &result->as_coercion;
  }

  /* // Add the edge now in case of recursion */
  /* if (append_edge(&induce->universe, source, target) == NULL) */
  /*   return NULL; */

  if (source->kind == MU_CORE_TYPE && target->kind == MU_CORE_TYPE) {
    // Add the edge now in case of recursion
    if (append_edge(&induce->universe, source, target) == NULL)
      return NULL;

    const mu_core_type_t *next_source = (const mu_core_type_t *) source;
    const mu_core_type_t *next_target = (const mu_core_type_t *) target;

    const mu_coercion_t *result;
    if ((result = ensure_core_coercion(induce, next_source, next_target)) == NULL)
      return NULL;

    universe_edge_t *edge;
    edge = universe_search(&induce->universe, source, target);
    assert(edge != NULL);
    edge->coercion = result;
    return result;
  }

  if (source->kind == MU_CORE_TYPE && target->kind == MU_VARIABLE_TYPE) {
    return ensure_cv(induce, source, target);
  }

  if (source->kind == MU_VARIABLE_TYPE && target->kind == MU_CORE_TYPE) {
    // Add the edge now in case of recursion
    if (append_edge(&induce->universe, source, target) == NULL)
      return NULL;

    universe_iterator_t iterator;
    const mu_type_t *next_source;

    // The target type isn't a variable type. For each edge:
    //   next_source => source | next_source isn't a variable type
    // Ensure that we're able to make the coercion:
    //   next_source => target

    iterator = universe_iterator(&induce->universe, source, 0);
    while ((next_source = universe_next(&iterator)) != NULL) {
      if (next_source->kind == MU_VARIABLE_TYPE)
        continue;
      if (ensure_coercion(induce, next_source, target) == NULL)
        return NULL;
    }

    // Then, for each edge:
    //   next_source => source | next_source is a variable type
    // Add the edge:
    //   next_source => target
    //
    // To maintain the transitive closure of variable types.

    iterator = universe_iterator(&induce->universe, source, 0);
    while ((next_source = universe_next(&iterator)) != NULL) {
      if (next_source->kind != MU_VARIABLE_TYPE)
        continue;
      append_edge(&induce->universe, next_source, target);
    }

    const mu_edge_coercion_t *result;
    if ((result = mu_edge_coercion(source, target)) == NULL)
      return NULL;
    return &result->as_coercion;
  }

  if (source->kind == MU_VARIABLE_TYPE && target->kind == MU_VARIABLE_TYPE) {
    // Add the edge now in case of recursion
    if (append_edge(&induce->universe, source, target) == NULL)
      return NULL;

    universe_iterator_t source_iterator;
    universe_iterator_t target_iterator;
    const mu_type_t *next_source, *next_target;

    // The target type isn't a variable type. For each edge:
    //   next_source => source | next_source isn't a variable type
    //
    // And for each edge:
    //   target => next_target | next_target isn't a variable type
    //
    // Ensure that we're able to make the coercion:
    //   next_source => next_target
    source_iterator = universe_iterator(&induce->universe, source, 0);
    while ((next_source = universe_next(&source_iterator)) != NULL) {
      if (next_source->kind == MU_VARIABLE_TYPE)
        continue;

      target_iterator = universe_iterator(&induce->universe, target, 1);
      while ((next_target = universe_next(&target_iterator)) != NULL) {
        if (next_target->kind == MU_VARIABLE_TYPE)
          continue;

        if (ensure_coercion(induce, next_source, next_target) == NULL)
          return NULL;
      }
    }

    source_iterator = universe_iterator(&induce->universe, source, 0);
    while ((next_source = universe_next(&source_iterator)) != NULL)
      append_edge(&induce->universe, next_source, target);

    target_iterator = universe_iterator(&induce->universe, target, 1);
    while ((next_target = universe_next(&target_iterator)) != NULL)
      append_edge(&induce->universe, source, next_target);

    const mu_edge_coercion_t *result;
    if ((result = mu_edge_coercion(source, target)) == NULL)
      return NULL;
    return &result->as_coercion;
  }

  __builtin_unreachable();
}

static inline const mu_coercion_t *ensure_core_coercion(
    induce_t *induce, const mu_core_type_t *source, const mu_core_type_t *target) {
  const mu_core_t *source_core = source->core;
  const mu_core_t *target_core = target->core;

  if (source_core != target_core) {
    fprintf(stderr, "Type mismatch\n");
    abort();
  }

  const mu_core_t *core = source_core;

  mu_variance_coercion_t *allocation;
  if ((allocation = variance_coercion_allocate(core)) == NULL)
    return NULL;

  for (size_t i = 0; i < core->argc; i++) {
    const mu_type_t *next_source = source->argv[i];
    const mu_type_t *next_target = target->argv[i];

    mu_variance_t variance = core->argv[i].variance;
    assert(variance != MU_INVARIANCE);
    if (variance == MU_CONTRAVARIANCE) {
      const mu_type_t *t = next_source; next_source = next_target; next_target = t;
    }

    const mu_coercion_t *coercion;
    if ((coercion = ensure_coercion(induce, next_source, next_target)) == NULL)
      return NULL;
    allocation->argv[i] = coercion;
  }

  const mu_variance_coercion_t *result;
  if ((result = variance_coercion_activate(allocation)) == NULL)
    return NULL;
  return &result->as_coercion;
}

static inline const mu_coercion_t *ensure_cv(
    induce_t *induce, const mu_type_t *source, const mu_type_t *target) {
  universe_iterator_t iterator;
  const mu_type_t *next_target;

  // For each edge ⟨next_source ⇒ target⟩

  // If source => next_source for any next_source in the target's sources, then
  // just do that.
  //
  // Skip indirect edges. If we need to coerce source => target, we don't want
  // to go through an arbitrary source variable.
  iterator = universe_iterator(&induce->universe, target, 0);
  universe_edge_t *next_edge;
  while ((next_edge = universe_next_edge(&iterator)) != NULL) {
    if (next_edge->indirect)
      continue;
    if (next_edge->source->kind == MU_VARIABLE_TYPE)
      continue;

    if (next_edge->source == source)
      return induce->id_coercion;

    const mu_coercion_t *coercion;
    if ((coercion = retrieve_coercion(induce, source, next_edge->source)) == NULL)
      return NULL;
    if (coercion != NO_SUCH_COERCION) {
      universe_edge_t *edge;
      if ((edge = append_edge(&induce->universe, source, next_edge->source)) == NULL)
        return NULL;
      edge->coercion = coercion;

      /* if ((edge = append_edge(&induce->universe, source, target)) == NULL) */
      /*   return NULL; */
      /* edge->indirect = 1; */

      const mu_edge_coercion_t *result;
      if ((result = mu_edge_coercion(source, target)) == NULL)
        return NULL;
      return &result->as_coercion;
    }
  }

  // Add the edge now in case of recursion
  if (append_edge(&induce->universe, source, target) == NULL)
    return NULL;

  // Ensure the coercion:
  //   source => next_target
  // Where next_target is each (direct and indirect) target of the variable
  // type.
  iterator = universe_iterator(&induce->universe, target, 1);
  while ((next_target = universe_next(&iterator)) != NULL) {
    if (next_target->kind == MU_VARIABLE_TYPE)
      continue;
    if (ensure_coercion(induce, next_target, source) == NULL)
      return NULL;
  }

  iterator = universe_iterator(&induce->universe, target, 1);
  while ((next_target = universe_next(&iterator)) != NULL) {
    if (next_target->kind != MU_VARIABLE_TYPE)
      continue;
    append_edge(&induce->universe, next_target, source);
  }

  // Now, check to see if we can put an existing edge to this type
  iterator = universe_iterator(&induce->universe, target, 0);
  while ((next_edge = universe_next_edge(&iterator)) != NULL) {
    if (next_edge->indirect)
      continue;
    if (next_edge->source == source)
      continue;

    if (next_edge->source->kind == MU_VARIABLE_TYPE)
      continue;

    const mu_coercion_t *coercion;
    if ((coercion = retrieve_coercion(induce, next_edge->source, source)) == NULL)
      return NULL;
    if (coercion == NO_SUCH_COERCION)
      continue;
    next_edge->indirect = 1;

    universe_edge_t *edge;
    if ((edge = append_edge(&induce->universe, next_edge->source, source)) == NULL)
      return NULL;
    edge->coercion = coercion;
  }

  const mu_edge_coercion_t *result;
  if ((result = mu_edge_coercion(source, target)) == NULL)
    return NULL;
  return &result->as_coercion;
}




induce_t *induce_initialize(
    induce_t *induce,
    mu_engine_t *engine,
    mu_status_t *status,
    const detect_t *detect) {
  assert(detect_result(detect)->engine == engine);

  size_t node_length = engine->node_number;

  const mu_type_t **node_to_type;
  if ((node_to_type = malloc(sizeof(const mu_type_t *[node_length]))) == NULL)
    return NULL;
  for (size_t i = 0; i < node_length; node_to_type[i++] = NULL);

  universe_t universe;
  if (rare(universe_initialize(&universe) == NULL))
    return NULL;

  mu_coercion_t *id_coercion;
  if ((id_coercion = malloc(sizeof(mu_coercion_t))) == NULL)
    return NULL;
  *id_coercion = (mu_coercion_t) { .kind = MU_ID_COERCION };

  mu_core_t *boolean_core;
  if ((boolean_core = malloc(sizeof(mu_core_t))) == NULL)
    return NULL;
  *boolean_core = (mu_core_t) { .kind = MU_BOOLEAN_CORE, .induce = induce };

  mu_core_t *integer_core;
  if ((integer_core = malloc(sizeof(mu_core_t))) == NULL)
    return NULL;
  *integer_core = (mu_core_t) { .kind = MU_INTEGER_CORE, .induce = induce };

  size_t size;

  mu_core_t *lambda_core;
  size = struct_size(mu_core_t, argv, 2);
  if ((lambda_core = malloc(size)) == NULL)
    return NULL;
  *lambda_core = (mu_core_t) { .kind = MU_LAMBDA_CORE, .induce = induce, .argc = 2 };
  lambda_core->argv[0] = (mu_core_member_t) { .variance = MU_CONTRAVARIANCE };
  lambda_core->argv[1] = (mu_core_member_t) { .variance = MU_COVARIANCE };

  mu_core_t *vector_core;
  size = struct_size(mu_core_t, argv, 1);
  if ((vector_core = malloc(size)) == NULL)
    return NULL;
  *vector_core = (mu_core_t) { .kind = MU_VECTOR_CORE, .induce = induce, .argc = 1 };
  vector_core->argv[0] = (mu_core_member_t) { .variance = MU_COVARIANCE };

  *induce = (induce_t) {
    .engine = engine,
    .status = status,
    .detect = detect_result(detect),
    .node_length = node_length,
    .node_to_type = node_to_type,
    .universe = universe,

    .id_coercion = id_coercion,

    .boolean_core = boolean_core,
    .integer_core = integer_core,
    .lambda_core = lambda_core,
    .vector_core = vector_core,
  };
  return induce;
}
