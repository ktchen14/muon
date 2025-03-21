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

#define evince induce_reveal

_Thread_local induce_t *debug_induce;

const mu_name_t *vector_access;
const mu_name_t *vector_join;

static const induce_edge_t *restrict_type_semiinternal(
    induce_t *induce, const mu_type_t *a, const mu_type_t *b, _Bool direct);

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

static const tactic_t no_tactic = {0};

static const tactic_t *restrict_type_internal(
    induce_t *induce, const mu_type_t *a, const mu_type_t *b, _Bool direct) {
  assert(a->kind != MU_SCHEME_TYPE && b->kind != MU_SCHEME_TYPE);

  if (a->kind == MU_CORE_TYPE && b->kind == MU_CORE_TYPE) {
    const mu_core_type_t *core_type_a = (const mu_core_type_t *) a;
    const mu_core_t *core_a = core_type_a->core;

    const mu_core_type_t *core_type_b = (const mu_core_type_t *) b;
    const mu_core_t *core_b = core_type_b->core;

    if (core_a->kind == MU_RECORD_CORE && core_b->kind == MU_RECORD_CORE) {
      const record_instance_t *instance;
      if (rare((instance = get_record_instance(induce, core_a, core_b)) == NULL))
        return NULL;

      for (size_t j = 0; j < core_b->argc; j++) {
        size_t i = instance->argv[j];
        if (restrict_type_semiinternal(induce, core_type_a->argv[i], core_type_b->argv[j], direct) == NULL)
          return NULL;
      }

      const record_tactic_t *result;
      if ((result = record_tactic_create(instance)) == NULL)
        return NULL;
      return &result->as_tactic;
    }

    if (core_type_a->core != core_type_b->core) {
      fprintf(stderr, "Type mismatch\n");
      abort();
    }

    const mu_core_t *core = core_type_a->core;

    for (size_t i = 0; i < core->argc; i++) {
      const mu_type_t *source = core_type_a->argv[i];
      const mu_type_t *target = core_type_b->argv[i];

      mu_variance_t variance = core->argv[i].variance;
      assert(variance != MU_INVARIANCE);
      if (variance == MU_CONTRAVARIANCE) {
        const mu_type_t *t = source; source = target; target = t;
      }

      if (restrict_type_semiinternal(induce, source, target, direct) == NULL)
        return NULL;
    }

    const variance_tactic_t *result;
    if ((result = variance_tactic_create(core)) == NULL)
      return NULL;
    return &result->as_tactic;
  }

  if (a->kind != MU_VARIABLE_TYPE && b->kind != MU_VARIABLE_TYPE) {
    fprintf(stderr, "Type mismatch\n");
    abort();
  }

  if (a->kind == MU_VARIABLE_TYPE && b->kind == MU_VARIABLE_TYPE)
    direct = 0;

  const mu_variable_type_t *variable_a;
  if ((variable_a = mu_type_cast(a, variable_a)) != NULL) {
    for (size_t i = 0; i < induce->universe.length; i++) {
      if (induce->universe.data[i].target != &variable_a->as_type)
        continue;
      if (restrict_type_semiinternal(induce, induce->universe.data[i].source, b, direct) == NULL)
        return NULL;
    }
  }

  const mu_variable_type_t *variable_b;
  if ((variable_b = mu_type_cast(b, variable_b)) != NULL) {
    for (size_t j = 0; j < induce->universe.length; j++) {
      if (induce->universe.data[j].source != &variable_b->as_type)
        continue;
      if (restrict_type_semiinternal(induce, a, induce->universe.data[j].target, direct) == NULL)
        return NULL;
    }
  }

  return &no_tactic;
}

const induce_edge_t SELF = {0};

static const induce_edge_t *restrict_type_semiinternal(
    induce_t *induce, const mu_type_t *a, const mu_type_t *b, _Bool direct) {
  if (a == b)
    return &SELF;

  // If we already have an edge a -> b then just return it
  const induce_edge_t *edge;
  if ((edge = universe_search(&induce->universe, a, b)) != NULL)
    return edge;

  const tactic_t *tactic;
  if ((tactic = restrict_type_internal(induce, a, b, direct)) == NULL)
    return NULL;
  if (tactic == &no_tactic)
    tactic = NULL;

  return universe_append(&induce->universe, a, b, direct, tactic);
}

const induce_edge_t *restrict_type(
    induce_t *induce, const mu_type_t *a, const mu_type_t *b) {
  return restrict_type_semiinternal(induce, a, b, 1);
}
