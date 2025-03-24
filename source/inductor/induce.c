#include "induce.h"

#include "../common.h"
#include "../stator.h"
#include "../status.h"
#include "coercion.h"
#include "core.h"
#include "detect.h"
#include "type.h"
#include "universe.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

_Thread_local induce_t *debug_induce;

/// @internal Ensure and return the coercion @a source ⇒ @a target
static const mu_coercion_t *ensure_cc(
    induce_t *induce, const mu_core_type_t *source, const mu_core_type_t *target);

/// @internal Ensure and return the coercion @a source ⇒ @a target
static const mu_coercion_t *ensure_cv(
    induce_t *induce, const mu_type_t *source, const mu_type_t *target);

/// @internal Ensure and return the coercion @a source ⇒ @a target
static const mu_coercion_t *ensure_vc(
    induce_t *induce, const mu_type_t *source, const mu_type_t *target);

static const mu_coercion_t *retrieve_core_coercion(
    induce_t *induce, const mu_core_type_t *source, const mu_core_type_t *target) {
  const mu_core_t *source_core = source->core;
  const mu_core_t *target_core = target->core;

  if (source_core->kind == MU_INTEGER_CORE && target_core->kind == MU_RECORD_CORE)
    return induce->id_coercion;

  if (source_core->kind == MU_BOOLEAN_CORE && target_core->kind == MU_INTEGER_CORE)
    return induce->id_coercion;

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

    const mu_coercion_t *coercion = retrieve_coercion(induce, next_source, next_target);
    if (coercion == NULL || coercion == NO_SUCH_COERCION) {
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

  // If we already have an edge source ⇒ target then
  const universe_edge_t *edge;
  if ((edge = universe_search(&induce->universe, source, target)) != NULL)
    return edge_to_coercion(edge);

  if (source->kind == MU_CORE_TYPE && target->kind == MU_CORE_TYPE) {
    const mu_core_type_t *next_source = (const mu_core_type_t *) source;
    const mu_core_type_t *next_target = (const mu_core_type_t *) target;

    const mu_coercion_t *result;
    if ((result = retrieve_core_coercion(induce, next_source, next_target)) == NULL)
      return NULL;

    if (result != NO_SUCH_COERCION) {
      universe_edge_t *edge;
      if ((edge = append_edge(&induce->universe, source, target)) == NULL)
        return NULL;
      edge->coercion = result;
    }
    return result;
  }

  return NO_SUCH_COERCION;
}

const mu_coercion_t *ensure_coercion(
    induce_t *induce, const mu_type_t *source, const mu_type_t *target) {
  assert(source->kind != MU_SCHEME_TYPE && target->kind != MU_SCHEME_TYPE);

  if (source == target)
    return induce->id_coercion;

  // If we already have an edge source ⇒ target then
  const universe_edge_t *edge;
  if ((edge = universe_search(&induce->universe, source, target)) != NULL)
    return edge_to_coercion(edge);

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
    if ((result = ensure_cc(induce, next_source, next_target)) == NULL)
      return NULL;

    universe_edge_t *edge;
    edge = universe_search(&induce->universe, source, target);
    assert(edge != NULL);
    edge->coercion = result;
    return result;
  }

  if (source->kind == MU_CORE_TYPE && target->kind == MU_VARIABLE_TYPE)
    return ensure_cv(induce, source, target);

  if (source->kind == MU_VARIABLE_TYPE && target->kind == MU_CORE_TYPE)
    return ensure_vc(induce, source, target);

  if (source->kind == MU_VARIABLE_TYPE && target->kind == MU_VARIABLE_TYPE) {
    // Add the edge now in case of recursion
    universe_edge_t *edge;
    if ((edge = append_edge(&induce->universe, source, target)) == NULL)
      return NULL;

    universe_iterator_t source_iterator;
    universe_iterator_t target_iterator;
    const mu_type_t *next_source, *next_target;

    // The target type isn't a variable type. For each edge:
    //   next_source ⇒ source | next_source isn't a variable type
    //
    // And for each edge:
    //   target ⇒ next_target | next_target isn't a variable type
    //
    // Ensure that we're able to make the coercion:
    //   next_source ⇒ next_target
    source_iterator = universe_iterator(&induce->universe, source, 0);
    while ((next_source = universe_next_type(&source_iterator)) != NULL) {
      if (next_source->kind == MU_VARIABLE_TYPE)
        continue;

      target_iterator = universe_iterator(&induce->universe, target, 1);
      while ((next_target = universe_next_type(&target_iterator)) != NULL) {
        if (next_target->kind == MU_VARIABLE_TYPE)
          continue;

        if (ensure_coercion(induce, next_source, next_target) == NULL)
          return NULL;

        universe_search(&induce->universe, next_source, next_target)->indirect = 2;
      }
    }

    source_iterator = universe_iterator(&induce->universe, source, 0);
    while ((next_source = universe_next_type(&source_iterator)) != NULL)
      append_edge(&induce->universe, next_source, target)->indirect = 1;

    target_iterator = universe_iterator(&induce->universe, target, 1);
    while ((next_target = universe_next_type(&target_iterator)) != NULL)
      append_edge(&induce->universe, source, next_target)->indirect = 1;

    const mu_edge_coercion_t *result;
    if ((result = mu_edge_coercion(source, target)) == NULL)
      return NULL;
    return edge->coercion = &result->as_coercion;
  }

  __builtin_unreachable();
}

static const mu_coercion_t *ensure_cc(
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

static const mu_coercion_t *ensure_cv(
    induce_t *induce, const mu_type_t *source, const mu_type_t *target) {
  typedef universe_edge_t edge_t;
  universe_iterator_t iterator;

  // If source <: x <: target, then:
  //
  //   (source ⇒ target) = (source ⇒ x) ∘ (x ⇒ target)
  //
  // In this case, record ⟨source ⇒ target⟩ as an indirect edge with an indirect
  // coercion through x.
  //
  // This allows us to skip adding source to the lower bound of target at all,
  // so that future calls to ensure_coercion(induce, target, something_else)
  // don't have to check another type, simplifies reduction, etc.
  //
  // We'll skip any subtype x of target that's also a variable type, as well as
  // if ⟨x ⇒ target⟩ is an indirect edge. This is because we don't want the
  // coercion source ⇒ target to occur through some variable type unrelated to
  // the context in which we're ensuring this coercion.
  iterator = universe_iterator(&induce->universe, target, 0);
  for (const edge_t *edge; (edge = universe_next(&iterator)) != NULL;) {
    if (edge->indirect || edge->source->kind == MU_VARIABLE_TYPE)
      continue;

    const mu_coercion_t *coercion;
    if ((coercion = retrieve_coercion(induce, source, edge->source)) == NULL)
      return NULL;

    if (coercion == NO_SUCH_COERCION)
      continue;

    const mu_coercion_t *tail;
    if ((tail = edge_to_coercion(edge)) == NULL)
      return NULL;

    const mu_indirect_coercion_t *result;
    if ((result = mu_indirect_coercion(coercion, tail)) == NULL)
      return NULL;

    universe_edge_t *edge;
    if ((edge = append_edge(&induce->universe, source, target)) == NULL)
      return NULL;
    edge->indirect = 2;

    return edge->coercion = &result->as_coercion;
  }

  // First, add ⟨source ⇒ target⟩ in case of recursion
  edge_t *result_edge;
  if ((result_edge = append_edge(&induce->universe, source, target)) == NULL)
    return NULL;

  // Then, ∀(next_target) | ∃⟨target ⇒ next_target⟩, ensure the coercion:
  //   source ⇒ next_target
  iterator = universe_iterator(&induce->universe, target, 1);
  for (const edge_t *edge; (edge = universe_next(&iterator)) != NULL;) {
    const mu_type_t *next_target = edge->target;
    if (edge->indirect > 1 || next_target->kind == MU_VARIABLE_TYPE)
      continue;
    if (ensure_coercion(induce, source, next_target) == NULL)
      return NULL;
    // TODO
    universe_search(&induce->universe, source, next_target)->indirect = 2;
  }

  // Then, ∀(next_target) | ∃⟨target ⇒ next_target⟩ where next_target is a
  // variable type, add ⟨source ⇒ next_target⟩ to maintain the transitive
  // closure of variable types in the universe.
  iterator = universe_iterator(&induce->universe, target, 1);
  for (const edge_t *edge; (edge = universe_next(&iterator)) != NULL;) {
    const mu_type_t *next_target = edge->target;
    if (next_target->kind != MU_VARIABLE_TYPE)
      continue;

    edge_t *next_edge;
    if ((next_edge = append_edge(&induce->universe, source, next_target)) == NULL)
      return NULL;
    next_edge->indirect = 1;
  }

  const mu_edge_coercion_t *result;
  if ((result = mu_edge_coercion(source, target)) == NULL)
    return NULL;
  result_edge->coercion = &result->as_coercion;

  // Once we've committed to ⟨source ⇒ target⟩, try to simplify target:
  //
  //   ∀(next_source) | ∃⟨next_source ⇒ target⟩, next_source ≠ source
  //
  // Retrieve next_source ⇒ source. If this exists, then add
  // ⟨next_source ⇒ source⟩ and make ⟨next_source ⇒ target⟩ an indirect edge.
  iterator = universe_iterator(&induce->universe, target, 0);
  for (edge_t *edge; (edge = universe_next(&iterator)) != result_edge;) {
    if (edge->indirect || edge->source->kind == MU_VARIABLE_TYPE)
      continue;
    if (edge->coercion != NULL && edge->coercion->kind != MU_EDGE_COERCION)
      continue;

    const mu_coercion_t *coercion;
    if ((coercion = retrieve_coercion(induce, edge->source, source)) == NULL)
      return NULL;

    if (coercion == NO_SUCH_COERCION)
      continue;

    if (edge->coercion == NULL) {
      const mu_indirect_coercion_t *new;
      if ((new = mu_indirect_coercion(coercion, &result->as_coercion)) == NULL)
        return NULL;
      edge->coercion = &new->as_coercion;
    } else {
      mu_indirect_coercion_t new = {
        .as_coercion = { .kind = MU_INDIRECT_COERCION, },
        .head = coercion,
        .tail = &result->as_coercion,
      };

      mu_indirect_coercion_t *over = (mu_indirect_coercion_t *) edge->coercion;
      memcpy(over, &new, sizeof(new));
    }

    edge->indirect = 2;
  }

  return &result->as_coercion;
}

static const mu_coercion_t *ensure_vc(
    induce_t *induce, const mu_type_t *source, const mu_type_t *target) {
  typedef universe_edge_t edge_t;

  // Add the edge now in case of recursion
  universe_edge_t *result_edge;
  if ((result_edge = append_edge(&induce->universe, source, target)) == NULL)
    return NULL;

  universe_iterator_t iterator;

  // The target type isn't a variable type. For each edge:
  //   next_source ⇒ source | next_source isn't a variable type
  // Ensure that we're able to make the coercion:
  //   next_source ⇒ target

  iterator = universe_iterator(&induce->universe, source, 0);
  for (const edge_t *edge; (edge = universe_next(&iterator)) != NULL;) {
    const mu_type_t *next_source = edge->source;
    if (edge->indirect > 1 || next_source->kind == MU_VARIABLE_TYPE)
      continue;
    if (ensure_coercion(induce, next_source, target) == NULL)
      return NULL;
    universe_search(&induce->universe, next_source, target)->indirect = 2;
  }

  // Then, for each edge:
  //   next_source ⇒ source | next_source is a variable type
  // Add the edge:
  //   next_source ⇒ target
  //
  // To maintain the transitive closure of variable types.

  iterator = universe_iterator(&induce->universe, source, 0);
  for (const edge_t *edge; (edge = universe_next(&iterator)) != NULL;) {
    const mu_type_t *next_source = edge->source;
    if (next_source->kind != MU_VARIABLE_TYPE)
      continue;

    edge_t *next_edge;
    if ((next_edge = append_edge(&induce->universe, next_source, target)) == NULL)
      return NULL;
    next_edge->indirect = 1;
  }

  const mu_edge_coercion_t *result;
  if ((result = mu_edge_coercion(source, target)) == NULL)
    return NULL;
  return result_edge->coercion = &result->as_coercion;
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
