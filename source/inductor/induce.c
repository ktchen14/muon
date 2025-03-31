#include "induce.h"

#include "../common.h"
#include "../stator.h"
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

  /* if (source_core->kind == MU_INTEGER_CORE && target_core->kind == MU_RECORD_CORE) */
  /*   return induce->id_coercion; */

  /* if (source_core->kind == MU_BOOLEAN_CORE && target_core->kind == MU_INTEGER_CORE) */
  /*   return induce->id_coercion; */

  if (source_core != target_core)
    return NO_SUCH_COERCION;

  const mu_core_t *core = source_core;

  mu_variance_coercion_t *allocation;
  if ((allocation = variance_coercion_allocate(target)) == NULL)
    return NULL;

  for (size_t i = 0; i < core->argc; i++) {
    const mu_type_t *next_source = source->argv[i];
    const mu_type_t *next_target = target->argv[i];

    mu_variance_t variance = core->argv[i].variance;
    assert(variance != MU_INVARIANCE);
    if (variance == MU_CONTRAVARIANCE) {
      const mu_type_t *t;
      t = next_source; next_source = next_target; next_target = t;
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

  // If ∃⟨source ⇒ target⟩ then return the coercion on that edge
  const type_edge_t *edge;
  if ((edge = universe_search(&induce->universe, source, target)) != NULL)
    return coerce_with(edge);

  if (source->kind == MU_CORE_TYPE && target->kind == MU_CORE_TYPE) {
    const mu_core_type_t *next_source = (const mu_core_type_t *) source;
    const mu_core_type_t *next_target = (const mu_core_type_t *) target;

    const mu_coercion_t *result;
    if ((result = retrieve_core_coercion(induce, next_source, next_target)) == NULL)
      return NULL;

    if (result == NO_SUCH_COERCION)
      return result;

    type_edge_t *edge;
    if ((edge = append_edge(&induce->universe, source, target)) == NULL)
      return NULL;
    return edge_assign(edge, result);
  }

  return NO_SUCH_COERCION;
}

const mu_coercion_t *ensure_coercion(
    induce_t *induce, const mu_type_t *source, const mu_type_t *target) {
  assert(source->kind != MU_SCHEME_TYPE && target->kind != MU_SCHEME_TYPE);

  if (source == target)
    return induce->id_coercion;

  // If ∃⟨source ⇒ target⟩, then return the coercion on that edge
  const type_edge_t *edge;
  if ((edge = universe_search(&induce->universe, source, target)) != NULL)
    return coerce_with(edge);

  if (source->kind == MU_CORE_TYPE && target->kind == MU_CORE_TYPE) {
    const mu_core_type_t *next_source = (const mu_core_type_t *) source;
    const mu_core_type_t *next_target = (const mu_core_type_t *) target;
    return ensure_cc(induce, next_source, next_target);
  }

  if (source->kind == MU_CORE_TYPE && target->kind == MU_VARIABLE_TYPE)
    return ensure_cv(induce, source, target);

  if (source->kind == MU_VARIABLE_TYPE && target->kind == MU_CORE_TYPE)
    return ensure_vc(induce, source, target);

  if (source->kind == MU_VARIABLE_TYPE && target->kind == MU_VARIABLE_TYPE) {
    // Add the edge now in case of recursion
    type_edge_t *edge;
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
      }
    }

    source_iterator = universe_iterator(&induce->universe, source, 0);
    while ((next_source = universe_next_type(&source_iterator)) != NULL)
      append_edge(&induce->universe, next_source, target)->transitive = 1;

    target_iterator = universe_iterator(&induce->universe, target, 1);
    while ((next_target = universe_next_type(&target_iterator)) != NULL)
      append_edge(&induce->universe, source, next_target)->transitive = 1;

    return coerce_with(edge);
  }

  __builtin_unreachable();
}

static const mu_coercion_t *ensure_cc(
    induce_t *induce, const mu_core_type_t *source, const mu_core_type_t *target) {
  // Add the edge now in case of recursion
  type_edge_t *result_edge;
  if ((result_edge = append_edge(&induce->universe, &source->as_type, &target->as_type)) == NULL)
    return NULL;

  const mu_core_t *source_core = source->core;
  const mu_core_t *target_core = target->core;

  if (source_core != target_core) {
    fprintf(stderr, "Type mismatch. Expected ");
    mu_core_debug(target_core);
    fprintf(stderr, " but got ");
    mu_core_debug(source_core);
    fprintf(stderr, "\n");
    abort();
  }

  const mu_core_t *core = source_core;

  mu_variance_coercion_t *allocation;
  if ((allocation = variance_coercion_allocate(target)) == NULL)
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
  return edge_assign(result_edge, &result->as_coercion);
}

static const mu_coercion_t *ensure_cv(
    induce_t *induce, const mu_type_t *source, const mu_type_t *target) {
  typedef type_edge_t edge_t;
  universe_iterator_t iterator;

  // Make the edge ⟨source ⇒ target⟩ in case of recursion
  edge_t *result_edge;
  if ((result_edge = append_edge(&induce->universe, source, target)) == NULL)
    return NULL;

  // Then, ∀(next_target) | ∃⟨target ⇒ next_target⟩, ensure the coercion:
  //   source ⇒ next_target
  iterator = universe_iterator(&induce->universe, target, 1);
  for (const edge_t *edge; (edge = universe_next(&iterator)) != NULL;) {
    const mu_type_t *next_target = edge->target;
    if (edge->indirect || next_target->kind == MU_VARIABLE_TYPE)
      continue;
    if (ensure_coercion(induce, source, next_target) == NULL)
      return NULL;
  }

  // Then, ∀(next_target) | ∃⟨target ⇒ next_target⟩ where next_target is a
  // variable type, add ⟨source ⇒ next_target⟩ to maintain the transitive
  // closure of variable types in the universe.
  iterator = universe_iterator(&induce->universe, target, 1);
  for (const edge_t *edge; (edge = universe_next(&iterator)) != NULL;) {
    if (edge->target->kind != MU_VARIABLE_TYPE)
      continue;

    edge_t *next;
    if ((next = append_edge(&induce->universe, source, edge->target)) == NULL)
      return NULL;
    next->transitive = 1;
  }

  return coerce_with(result_edge);
}

static const mu_coercion_t *ensure_vc(
    induce_t *induce, const mu_type_t *source, const mu_type_t *target) {
  // Add the edge now in case of recursion
  type_edge_t *result_edge;
  if ((result_edge = append_edge(&induce->universe, source, target)) == NULL)
    return NULL;

  universe_iterator_t iterator;

  // The target type isn't a variable type. For each edge:
  //   next_source ⇒ source | next_source isn't a variable type
  // Ensure that we're able to make the coercion:
  //   next_source ⇒ target

  iterator = universe_iterator(&induce->universe, source, 0);
  for (const type_edge_t *edge; (edge = universe_next(&iterator)) != NULL;) {
    const mu_type_t *next_source = edge->source;
    if (edge->indirect || next_source->kind == MU_VARIABLE_TYPE)
      continue;
    if (ensure_coercion(induce, next_source, target) == NULL)
      return NULL;
  }

  // Then, for each edge:
  //   next_source ⇒ source | next_source is a variable type
  // Add the edge:
  //   next_source ⇒ target
  //
  // To maintain the transitive closure of variable types.

  iterator = universe_iterator(&induce->universe, source, 0);
  for (const type_edge_t *edge; (edge = universe_next(&iterator)) != NULL;) {
    const mu_type_t *next_source = edge->source;
    if (next_source->kind != MU_VARIABLE_TYPE)
      continue;

    type_edge_t *next_edge;
    if ((next_edge = append_edge(&induce->universe, next_source, target)) == NULL)
      return NULL;
    next_edge->transitive = 1;
  }

  return coerce_with(result_edge);
}



static void mark_type(induce_t *induce, const mu_type_t *type, _Bool negative, size_t rank) {
  switch ON_ABSTRACT_OBJECT(type) {
    case IS_KIND_OF(core_type): {
      const mu_core_t *core = core_type->core;

      for (size_t i = 0; i < core->argc; i++) {
        const mu_type_t *next = core_type->argv[i];

        _Bool next_negative = negative;
        mu_variance_t variance = core->argv[i].variance;
        assert(variance != MU_INVARIANCE);
        if (variance == MU_CONTRAVARIANCE)
          next_negative = !next_negative;

        mark_type(induce, next, next_negative, rank);
      }

      break;
    }

    case IS_KIND_OF(variable_type):
      if (variable_type->rank < rank)
        return;

      ((mu_variable_type_t *) variable_type)->reachable[negative] = 1;

      universe_iterator_t it;
      it = universe_iterator(&induce->universe, &variable_type->as_type, negative);
      for (type_edge_t *edge; (edge = universe_next(&it)) != NULL;)
        mark_type(induce, edge->vertex[negative], negative, rank);

      break;

    case MU_SCHEME_TYPE:
      abort();
  }
}



const mu_type_t *generalize_type(
    induce_t *induce, const mu_type_t *matter, open_scheme_t *scheme) {
  mark_type(induce, matter, 0, scheme->rank);

  size_t polymorphic_length = 0;
  mu_variable_type_t *polymorphic = NULL;

  mu_variable_type_t *variable_type = scheme->link;
  while (variable_type != NULL) {
    assert(variable_type->rank == scheme->rank);

    mu_variable_type_t *next = variable_type->scheme_next;

    /* Does a type have to be both positively reachable and negatively reachable
     * from the type of the defined expr to be polymorphic? */

    /*
     * Not sure if this is true, but here are some thoughts:
     *
     * A variable type must be constrained somehow to be polymorphically useful.
     * If we have:
     *   foo :: a
     * Then, while theoretically foo is polymorphic, it's not any more useful
     * than:
     *   foo :: ⊥
     *
     * Similarly, this function:
     *   bar :: a -> ()
     * While theoretically polymorphic, is no more useful than:
     *   bar :: ⊤ -> ()
     *
     * A variable can be constrained by either appearing both positively and
     * negatively, being constrained by bounds, or (in the future) being
     * constrained by kind. For now, just do this:
     */
    if (variable_type->reachable[0] && variable_type->reachable[1]) {
      variable_type->scheme_next = polymorphic;
      polymorphic = variable_type;
      variable_type->rank = 0;
      polymorphic_length++;
    } else {
      variable_type->scheme_next = scheme->parent->link;
      scheme->parent->link = variable_type;
      variable_type->rank--;
    }

    variable_type = next;
  }

  if (polymorphic_length == 0)
    return matter;

  mu_scheme_type_t *allocation;
  if ((allocation = scheme_type_allocate(induce, polymorphic_length)) == NULL)
    return NULL;

  size_t i = 0;
  for (mu_variable_type_t *type = polymorphic; type != NULL; type = type->scheme_next) {
    allocation->argv[i++] = type;
    type->polymorphic_to = allocation;
  }

  const mu_scheme_type_t *result;
  if (rare((result = scheme_type_activate(allocation, matter)) == NULL))
    return NULL;
  return &result->as_type;
}

typedef struct {
  const mu_type_t *source;
  const mu_type_t *target;
} cache_item;

const mu_type_t *instantiate_single_type(
    induce_t *induce,
    const mu_type_t *type,
    const mu_scheme_type_t *scheme,
    open_scheme_t *target_scheme,
    cache_item *cache,
    size_t *cache_i
) {
  for (size_t i = 0; i < 100; i++) {
    if (cache[i].source == type)
      return cache[i].target;
  }

  switch ON_ABSTRACT_OBJECT(type) {
    case IS_KIND_OF(core_type): {
      const mu_core_t *core = core_type->core;

      if (core->argc == 0) {
        cache[(*cache_i)++] = (cache_item) { &core_type->as_type, &core_type->as_type };
        return &core_type->as_type;
      }

      mu_core_type_t *allocation;
      if ((allocation = core_type_allocate(induce, core)) == NULL)
        return NULL;

      _Bool same = 1;
      for (size_t i = 0; i < core->argc; i++) {
        allocation->argv[i] = instantiate_single_type(induce, core_type->argv[i], scheme, target_scheme, cache, cache_i);
        same = same && (allocation->argv[i] == core_type->argv[i]);
      }

      if (same) {
        free(allocation);
        cache[(*cache_i)++] = (cache_item) { &core_type->as_type, &core_type->as_type };
        return &core_type->as_type;
      }

      const mu_core_type_t *result = core_type_activate(allocation);
      cache[(*cache_i)++] = (cache_item) { &core_type->as_type, &result->as_type };
      return &result->as_type;
    }

    case IS_KIND_OF(variable_type):
      if (variable_type->polymorphic_to != scheme) {
        cache[(*cache_i)++] = (cache_item) { &variable_type->as_type, &variable_type->as_type };
        return &variable_type->as_type;
      }

      const mu_variable_type_t *newvar;
      if ((newvar = mu_variable_type(induce, target_scheme)) == NULL)
        return NULL;
      cache[(*cache_i)++] = (cache_item) { &variable_type->as_type, &newvar->as_type };

      universe_iterator_t it;
      it = universe_iterator(&induce->universe, &variable_type->as_type, 0);
      for (const type_edge_t *edge; (edge = universe_next(&it)) != NULL;) {
        const mu_type_t *next = instantiate_single_type(induce, edge->source, scheme, target_scheme, cache, cache_i);
        append_edge(&induce->universe, next, &newvar->as_type);
      }

      it = universe_iterator(&induce->universe, &variable_type->as_type, 1);
      for (const type_edge_t *edge; (edge = universe_next(&it)) != NULL;) {
        const mu_type_t *next = instantiate_single_type(induce, edge->target, scheme, target_scheme, cache, cache_i);
        append_edge(&induce->universe, &newvar->as_type, next);
      }

      cache[(*cache_i)++] = (cache_item) { &variable_type->as_type, &newvar->as_type };
      return &newvar->as_type;

    case MU_SCHEME_TYPE:
      fprintf(stderr, "Unsupported higher rank polymorphism\n");
      abort();
  }
}

const mu_type_t *instantiate_scheme(
    induce_t *induce, const mu_scheme_type_t *scheme_type, open_scheme_t *target_scheme
) {
  cache_item cache[100] = {0};
  size_t i = 0;
  return instantiate_single_type(induce, scheme_type->matter, scheme_type, target_scheme, cache, &i);
}




induce_t *induce_initialize(
    induce_t *induce, mu_engine_t *engine, const detect_t *detect) {
  assert(detect_result(detect)->engine == engine);

  size_t node_length = engine->node_number;

  const mu_type_t **node_to_type;
  if ((node_to_type = malloc(sizeof(const mu_type_t *[node_length]))) == NULL)
    return NULL;
  for (size_t i = 0; i < node_length; node_to_type[i++] = NULL);

  const mu_coercion_t **node_to_coercion;
  if ((node_to_coercion = malloc(sizeof(const mu_coercion_t *[node_length]))) == NULL)
    return NULL;
  for (size_t i = 0; i < node_length; node_to_coercion[i++] = NULL);

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
    .detect = detect_result(detect),
    .node_length = node_length,
    .node_to_type = node_to_type,
    .node_to_coercion = node_to_coercion,
    .universe = universe,

    .id_coercion = &id_coercion->as_coercion,

    .boolean_core = boolean_core,
    .integer_core = integer_core,
    .lambda_core = lambda_core,
    .vector_core = vector_core,
  };
  return induce;
}
