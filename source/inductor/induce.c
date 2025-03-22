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

/* static const induce_edge_t *restrict_type_semiinternal( */
/*     induce_t *induce, const mu_type_t *a, const mu_type_t *b, _Bool direct); */

const mu_variable_type_t *variable_type(induce_t *induce, open_scheme_t *scheme) {
  mu_variable_type_t *result;
  if ((result = malloc(sizeof(mu_variable_type_t))) == NULL)
    return NULL;

  *result = (mu_variable_type_t) {
    .as_type.kind = MU_VARIABLE_TYPE,
    .as_type.induce = induce,
    .as_type.id = induce->type_number++,
    .scheme_next = scheme->link,
    .rank = scheme->rank,
  };
  return scheme->link = result;
}

open_scheme_t *open_scheme(open_scheme_t *parent, const mu_node_t *node) {
  open_scheme_t *result;
  if ((result = malloc(sizeof(open_scheme_t))) == NULL)
    return NULL;
  *result = (open_scheme_t) {
    .induce = parent->induce, .node = node, .parent = parent, .rank = parent->rank + 1,
  };
  return result;
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

  mu_id_coercion_t *id_coercion;
  if ((id_coercion = malloc(sizeof(mu_id_coercion_t))) == NULL)
    return NULL;
  *id_coercion = (mu_id_coercion_t) { .as_coercion.kind = MU_ID_COERCION };

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

const mu_coercion_t *universe_get_coercion(
    const universe_t *universe, const mu_type_t *source, const mu_type_t *target) {
  const universe_edge_t *edge;
  if ((edge = universe_search(universe, source, target)) == NULL)
    return NULL;
  return edge->coercion;
}

const mu_coercion_t *make_coercion(
    induce_t *induce, const mu_type_t *source, const mu_type_t *target);

universe_edge_t *append_edge(universe_t *universe, const mu_type_t *source, const mu_type_t *target);

static inline const mu_coercion_t *make_core_coercion(
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
    if ((coercion = make_coercion(induce, next_source, next_target)) == NULL)
      return NULL;
    allocation->argv[i] = coercion;
  }

  const mu_variance_coercion_t *result;
  if ((result = variance_coercion_activate(allocation)) == NULL)
    return NULL;
  return &result->as_coercion;
}

const mu_coercion_t *make_coercion(
    induce_t *induce, const mu_type_t *source, const mu_type_t *target) {
  assert(source->kind != MU_SCHEME_TYPE && target->kind != MU_SCHEME_TYPE);

  if (source == target) {
  }

  // If we already have a coercion source => target, then just return it
  const mu_coercion_t *result;
  if ((result = universe_get_coercion(&induce->universe, source, target)) != NULL)
    return result;

  if (append_edge(&induce->universe, source, target) == NULL)
    return NULL;

  if (source->kind == MU_CORE_TYPE && target->kind == MU_CORE_TYPE) {
    const mu_core_type_t *next_source = (const mu_core_type_t *) source;
    const mu_core_type_t *next_target = (const mu_core_type_t *) target;

    const mu_coercion_t *result;
    if ((result = make_core_coercion(induce, next_source, next_target)) == NULL)
      return NULL;

    universe_edge_t *edge;
    edge = universe_search(&induce->universe, source, target);
    assert(edge != NULL);
    edge->coercion = result;
    return result;
  }

  if (source->kind == MU_VARIABLE_TYPE && target->kind == MU_CORE_TYPE) {
    const mu_variable_type_t *source_variable_type = (const mu_variable_type_t *) source;

    for (size_t i = 0; i < induce->universe.length; i++) {
      universe_edge_t edge = induce->universe.data[i];
      if (edge.target != &source_variable_type->as_type)
        continue;
      if (edge.source->kind == MU_VARIABLE_TYPE)
        continue;

      if (make_coercion(induce, edge.source, target) == NULL)
        return NULL;
    }

    for (size_t i = 0; i < induce->universe.length; i++) {
      universe_edge_t edge = induce->universe.data[i];
      if (edge.target != &source_variable_type->as_type)
        continue;
      if (edge.source->kind != MU_VARIABLE_TYPE)
        continue;

      append_edge(&induce->universe, edge.source, target);
    }

    const mu_coercion_t *result;
    if ((result = mu_edge_coercion(source, target)) == NULL)
      return NULL;
    return result;
  }

  if (source->kind == MU_CORE_TYPE && target->kind == MU_VARIABLE_TYPE) {
    const mu_variable_type_t *target_variable_type = (const mu_variable_type_t *) target;

    for (size_t i = 0; i < induce->universe.length; i++) {
      universe_edge_t edge = induce->universe.data[i];
      if (edge.source != &target_variable_type->as_type)
        continue;
      if (edge.target->kind == MU_VARIABLE_TYPE)
        continue;

      if (make_coercion(induce, source, edge.target) == NULL)
        return NULL;
    }

    for (size_t i = 0; i < induce->universe.length; i++) {
      universe_edge_t edge = induce->universe.data[i];
      if (edge.source != &target_variable_type->as_type)
        continue;
      if (edge.target->kind != MU_VARIABLE_TYPE)
        continue;

      append_edge(&induce->universe, source, edge.target);
    }

    const mu_coercion_t *result;
    if ((result = mu_edge_coercion(source, target)) == NULL)
      return NULL;
    return result;
  }

  if (source->kind == MU_VARIABLE_TYPE && target->kind == MU_VARIABLE_TYPE) {
    const mu_variable_type_t *source_variable_type = (const mu_variable_type_t *) source;
    const mu_variable_type_t *target_variable_type = (const mu_variable_type_t *) target;

    for (size_t i = 0; i < induce->universe.length; i++) {
      universe_edge_t source_edge = induce->universe.data[i];
      if (source_edge.target != &source_variable_type->as_type)
        continue;
      if (source_edge.target->kind == MU_VARIABLE_TYPE)
        continue;

      for (size_t i = 0; i < induce->universe.length; i++) {
        universe_edge_t target_edge = induce->universe.data[i];
        if (target_edge.source != &target_variable_type->as_type)
          continue;
        if (target_edge.source->kind == MU_VARIABLE_TYPE)
          continue;

        if (make_coercion(induce, source_edge.source, target_edge.target) == NULL)
          return NULL;
      }
    }

    for (size_t i = 0; i < induce->universe.length; i++) {
      universe_edge_t edge = induce->universe.data[i];
      if (edge.target != &source_variable_type->as_type)
        continue;
      if (edge.target->kind != MU_VARIABLE_TYPE)
        continue;

      for (size_t i = 0; i < induce->universe.length; i++) {
        universe_edge_t edge = induce->universe.data[i];
        if (edge.source != &target_variable_type->as_type)
          continue;
        if (edge.source->kind != MU_VARIABLE_TYPE)
          continue;
        assert(edge.source->kind != MU_SCHEME_TYPE);

        append_edge(&induce->universe, edge.source, edge.target);
      }
    }

    const mu_coercion_t *result;
    if ((result = mu_edge_coercion(source, target)) == NULL)
      return NULL;
    return result;
  }

  abort();
}

const induce_edge_t *restrict_type(
    induce_t *induce, const mu_type_t *source, const mu_type_t *target) {

  const mu_coercion_t *coercion;
  if ((coercion = make_coercion(induce, source, target)) == NULL)
    return NULL;

  return universe_search(&induce->universe, source, target);
}
