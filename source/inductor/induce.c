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

static const mu_type_t *instantiate_scheme(
    induce_t *induce, const mu_scheme_type_t *scheme)
  __attribute__((nonnull));

/// @internal Ensure and return the coercion @a source ⇝ @a target. Doesn't bail
/// when source is target, or when ∃⟨source ⇒ target⟩.
static const mu_coercion_t *ensure_static_coercion_internal(
    induce_t *induce,
    const mu_static_type_t *restrict source,
    const mu_static_type_t *restrict target)
  __attribute__((nonnull));

/// @internal Ensure and return the coercion @a source ⇝ @a target
static const mu_coercion_t *ensure_static_coercion(
    induce_t *induce,
    const mu_static_type_t *source,
    const mu_static_type_t *target)
  __attribute__((nonnull));

static const mu_coercion_t *retrieve_core_coercion(
    induce_t *induce, const mu_core_type_t *source, const mu_core_type_t *target);

const mu_coercion_t *ensure_coercion(
    induce_t *induce, const mu_type_t *source, const mu_type_t *target) {
  // If source is the same type as target, then just return the id coercion
  if (source == target)
    return induce->id_coercion;

  // If ∃⟨source ⇒ target⟩, then just return the coercion on that edge
  const type_edge_t *edge;
  if ((edge = universe_search(&induce->universe, source, target)) != NULL)
    return coerce_with(edge);

  const mu_static_type_t *static_source = mu_type_cast(source, static_source);
  const mu_static_type_t *static_target = mu_type_cast(target, static_target);

  if (static_source == NULL && static_target != NULL) {
    assert(source->kind == MU_VARIABLE_TYPE);

    // Make ⟨source ⇒ target⟩ here in case of recursion
    type_edge_t *result_edge;
    if ((result_edge = append_edge(&induce->universe, source, target)) == NULL)
      return NULL;

    universe_iterator_t it;

    // ∀(τ) | ∃⟨τ ⇒ source⟩ and τ isn't a variable type, ensure that we're able
    // to coerce τ ⇝ target
    it = universe_iterator(&induce->universe, source, 0);
    for (const type_edge_t *edge; (edge = universe_next(&it)) != NULL;) {
      const mu_static_type_t *t;
      if (edge->indirect || (t = mu_type_cast(edge->source, t)) == NULL)
        continue;

      if (ensure_static_coercion(induce, t, static_target) == NULL)
        return NULL;
    }

    // ∀(τ) | ∃⟨τ ⇒ source⟩ and τ is a variable type, create ⟨τ ⇒ target⟩ to
    // maintain the target-side transitive closure of τ
    it = universe_iterator(&induce->universe, source, 0);
    for (const type_edge_t *edge; (edge = universe_next(&it)) != NULL;) {
      const mu_type_t *t;
      if (edge->indirect || (t = edge->source)->kind != MU_VARIABLE_TYPE)
        continue;

      type_edge_t *addition;
      if ((addition = edge_define(&induce->universe, t, target)) == NULL)
        return NULL;
      addition->transitive = 1;
    }

    return coerce_with(result_edge);
  }

  if (static_source != NULL && static_target == NULL) {
    assert(target->kind == MU_VARIABLE_TYPE);

    // Make ⟨source ⇒ target⟩ here in case of recursion
    type_edge_t *result_edge;
    if ((result_edge = append_edge(&induce->universe, source, target)) == NULL)
      return NULL;

    universe_iterator_t it;

    // ∀(τ) | ∃⟨target ⇒ τ⟩ and τ isn't a variable type, ensure that we're able
    // to coerce source ⇝ τ
    it = universe_iterator(&induce->universe, target, 1);
    for (const type_edge_t *edge; (edge = universe_next(&it)) != NULL;) {
      const mu_static_type_t *t;
      if (edge->indirect || (t = mu_type_cast(edge->target, t)) == NULL)
        continue;

      if (ensure_static_coercion(induce, static_source, t) == NULL)
        return NULL;
    }

    // ∀(τ) | ∃⟨target ⇒ τ⟩ and τ is a variable type, create ⟨source ⇒ τ⟩ to
    // maintain the source-side transitive closure of τ
    it = universe_iterator(&induce->universe, target, 1);
    for (const type_edge_t *edge; (edge = universe_next(&it)) != NULL;) {
      const mu_type_t *t;
      if (edge->indirect || (t = edge->target)->kind != MU_VARIABLE_TYPE)
        continue;

      type_edge_t *addition;
      if ((addition = edge_define(&induce->universe, source, t)) == NULL)
        return NULL;
      addition->transitive = 1;
    }

    return coerce_with(result_edge);
  }

  if (static_source == NULL && static_target == NULL) {
    assert(source->kind == MU_VARIABLE_TYPE);
    assert(target->kind == MU_VARIABLE_TYPE);

    // Make ⟨source ⇒ target⟩ here in case of recursion
    type_edge_t *edge;
    if ((edge = append_edge(&induce->universe, source, target)) == NULL)
      return NULL;

    universe_iterator_t it, jt;

    // ∀(α) | ∃⟨α ⇒ source⟩ and α isn't a variable type, ∀(β) | ∃⟨target ⇒ β⟩
    // and β isn't a variable type, ensure that we're able to coerce α ⇝ β
    it = universe_iterator(&induce->universe, source, 0);
    for (const type_edge_t *a_edge; (a_edge = universe_next(&it)) != NULL;) {
      const mu_static_type_t *a;
      if (a_edge->indirect || (a = mu_type_cast(a_edge->source, a)) == NULL)
        continue;

      jt = universe_iterator(&induce->universe, target, 1);
      for (const type_edge_t *b_edge; (b_edge = universe_next(&jt)) != NULL;) {
        const mu_static_type_t *b;
        if (b_edge->indirect || (b = mu_type_cast(b_edge->target, b)) == NULL)
          continue;

        if (ensure_static_coercion(induce, a, b) == NULL)
          return NULL;
      }
    }

    // ∀(α) | ∃⟨α ⇒ source⟩, create ⟨α ⇒ target⟩ to maintain the target-side
    // transitive closure of α
    it = universe_iterator(&induce->universe, source, 0);
    for (const type_edge_t *edge; (edge = universe_next(&it)) != NULL;) {
      if (edge->indirect)
        continue;

      const mu_type_t *a = edge->source;
      type_edge_t *next_edge;
      if ((next_edge = edge_define(&induce->universe, a, target)) == NULL)
        return NULL;
      next_edge->transitive = 1;
    }

    // ∀(β) | ∃⟨target ⇒ β⟩, create ⟨source ⇒ β⟩ to maintain the source-side
    // transitive closure of β
    jt = universe_iterator(&induce->universe, target, 1);
    for (const type_edge_t *edge; (edge = universe_next(&it)) != NULL;) {
      if (edge->indirect)
        continue;

      const mu_type_t *b = edge->target;
      type_edge_t *next_edge;
      if ((next_edge = edge_define(&induce->universe, source, b)) == NULL)
        return NULL;
      next_edge->transitive = 1;
    }

    return coerce_with(edge);
  }

  assert(static_source != NULL);
  assert(static_target != NULL);
  return ensure_static_coercion_internal(induce, static_source, static_target);
}

static const mu_coercion_t *ensure_static_coercion(
    induce_t *induce,
    const mu_static_type_t *source,
    const mu_static_type_t *target) {
  // If source is the same type as target, then just return the id coercion
  if (source == target)
    return induce->id_coercion;

  // If ∃⟨source ⇒ target⟩, then just return the coercion on that edge
  const type_edge_t *edge;
  if ((edge = universe_search(&induce->universe, &source->as_type, &target->as_type)) != NULL)
    return coerce_with(edge);

  return ensure_static_coercion_internal(induce, source, target);
}

static const mu_coercion_t *ensure_static_coercion_internal(
    induce_t *induce,
    const mu_static_type_t *restrict source,
    const mu_static_type_t *restrict target) {
  assert(target->kind != MU_SCHEME_STATIC_TYPE);

  // Make ⟨source ⇒ target⟩ here in case of recursion
  type_edge_t *result_edge;
  if ((result_edge = append_edge(&induce->universe, &source->as_type, &target->as_type)) == NULL)
    return NULL;

  const mu_scheme_type_t *scheme_type;
  if ((scheme_type = mu_static_type_cast(source, scheme_type)) != NULL) {
    // Create an unscheme coercion source ⇝ instance to instantiate the scheme
    // type
    const mu_type_t *instance;
    if ((instance = instantiate_scheme(induce, scheme_type)) == NULL)
      return NULL;

    const mu_unscheme_coercion_t *head;
    if ((head = mu_unscheme_coercion()) == NULL)
      return NULL;

    type_edge_t *edge;
    if ((edge = append_edge(&induce->universe, &source->as_type, instance)) == NULL)
      return NULL;
    edge->coercion = &head->as_coercion;

    // Ensure that we're able to coerce instance ⇝ target
    const mu_coercion_t *coercion;
    if ((coercion = ensure_coercion(induce, instance, &target->as_type)) == NULL)
      return NULL;
    if (coercion == NO_SUCH_COERCION)
      return NO_SUCH_COERCION;

    // Then return the coercion (source ⇝ instance) ∘ (instance ⇝ target)
    const mu_indirect_coercion_t *result;
    if ((result = mu_indirect_coercion(&head->as_coercion, coercion)) == NULL)
      return NULL;
    return edge_assign(result_edge, &result->as_coercion);
  }

  const mu_core_type_t *core_source = mu_static_type_cast(source, core_source);
  assert(core_source != NULL);

  const mu_core_type_t *core_target = mu_static_type_cast(target, core_target);
  assert(core_target != NULL);

  const mu_core_t *source_core = core_source->core;
  const mu_core_t *target_core = core_target->core;

  if (source_core != target_core) {
    const mu_instance_t *instance;
    for (size_t i = 0; i < induce->instance_length; i++) {
      instance = induce->instance[i];
      if (instance->source != source_core)
        continue;
      if (instance->target != target_core)
        continue;
      goto instance_coercion;
    }

    fprintf(stderr, "Type mismatch. Expected ");
    mu_core_debug(target_core);
    fprintf(stderr, " but got ");
    mu_core_debug(source_core);
    fprintf(stderr, "\n");
    abort();

  instance_coercion:;
    const mu_instance_coercion_t *result;
    if ((result = mu_instance_coercion(instance)) == NULL)
      return NULL;
    return edge_assign(result_edge, &result->as_coercion);
  }

  const mu_core_t *core = source_core;

  mu_variance_coercion_t *allocation;
  if ((allocation = variance_coercion_allocate(core)) == NULL)
    return NULL;

  for (size_t i = 0; i < core->argc; i++) {
    const mu_type_t *next_source = core_source->argv[i];
    const mu_type_t *next_target = core_target->argv[i];

    mu_variance_t variance = core->argv[i].variance;
    assert(variance != MU_INVARIANCE);
    if (variance == MU_CONTRAVARIANCE) {
      const mu_type_t *t;
      t = next_source; next_source = next_target; next_target = t;
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



static void mark_type(induce_t *induce, const mu_type_t *type, _Bool negative, size_t rank) {
  switch ON_ABSTRACT_OBJECT(type) {
    case IS_KIND_OF(core_type): {
      const mu_core_t *core = core_type->core;

      for (size_t i = 0; i < core->argc; i++) {
        const mu_type_t *argument = core_type->argv[i];

        _Bool argn = negative;
        mu_variance_t variance = core->argv[i].variance;
        assert(variance != MU_INVARIANCE);
        if (variance == MU_CONTRAVARIANCE)
          argn = !argn;

        mark_type(induce, argument, argn, rank);
      }

      break;
    }

    case IS_KIND_OF(variable_type):
      if (variable_type->rank < rank)
        return;

      ((mu_variable_type_t *) variable_type)->reachable[negative] = 1;

      universe_iterator_t it;

      it = universe_iterator(&induce->universe, type, negative);
      for (type_edge_t *edge; (edge = universe_next(&it)) != NULL;) {
        const mu_type_t *vertex = edge->vertex[negative];

        const mu_variable_type_t *variable_vertex;
        if ((variable_vertex = mu_type_cast(vertex, variable_vertex)) == NULL)
          mark_type(induce, vertex, negative, rank);
        else if (variable_vertex->rank >= rank)
          ((mu_variable_type_t *) variable_vertex)->reachable[negative] = 1;
      }

      break;

    case IS_KIND_OF(scheme_type):
      mark_type(induce, scheme_type->matter, negative, rank);
      break;
  }
}

static void collect(
    induce_t *induce, const mu_type_t *type, _Bool negative, size_t rank,
    const mu_type_t **buffer, size_t *i) {
  switch ON_ABSTRACT_OBJECT(type) {
    case IS_KIND_OF(core_type): {
      const mu_core_t *core = core_type->core;

      for (size_t k = 0; k < core->argc; k++) {
        const mu_type_t *argument = core_type->argv[k];

        if (argument->semipolymorphic == 0) {
          _Bool argn = negative;
          mu_variance_t variance = core->argv[k].variance;
          assert(variance != MU_INVARIANCE);
          if (variance == MU_CONTRAVARIANCE)
            argn = !argn;

          collect(induce, argument, argn, rank, buffer, i);
        }

        if (argument->semipolymorphic == 1) {
          ((mu_type_t *) type)->semipolymorphic = 1;
          buffer[(*i)++] = type;
          break;
        }
      }

      break;
    }

    case IS_KIND_OF(variable_type):
      // This is a polymorphic root. It's already been collected.
      if (variable_type->rank == rank &&
          variable_type->reachable[0] &&
          variable_type->reachable[1]) {
        return;
      }

      universe_iterator_t it;

      it = universe_iterator(&induce->universe, type, negative);
      for (type_edge_t *edge; (edge = universe_next(&it)) != NULL;) {
        if (semipolymorphic(induce, edge->vertex[negative], negative, rank))
          return 1;
      }

      return 0;

    case MU_SCHEME_TYPE:
      return 0;
  }
}

const mu_type_t *generalize_type(
    induce_t *induce, const mu_type_t *matter, mu_scheme_t *scheme) {
  // Unpack the linked list of variable types into an array
  mu_variable_type_t *variable[1000] = {0};
  size_t variable_length = 0;

  for (mu_variable_type_t *v = scheme->link, *next; v != NULL; v = next) {
    assert(v->rank == scheme->rank);

    next = v->scheme_next;
    v->scheme_next = NULL;
    variable[variable_length++] = v;
  }

  // Next, mark each variable type in the scheme with whether it's accessible
  // from matter.
  mark_type(induce, matter, 0, scheme->rank);

  mu_variable_type_t *polymorphic[1000];
  size_t polymorphic_length = 0;

  // Each variable type that's both + and - accessible from matter is a
  // "polymorphic root". However, these aren't the only types that are
  // polymorphic. For example, in:
  //
  //   ∀(β, γ) in (foo: γ) ⊓ β → γ
  //
  // γ is a polymorphic root, but β is polymorphic as well. When we instantiate
  // this type, we have to copy both γ and β.
  //
  // So we'll mark each type that can reach a polymorphic root as polymorphic.

  // Here, each polymorphic root is a variable type with reachable[0],
  // reachable[1], and rank == scheme->rank. Unless all these conditions are
  // met, a type isn't a polymorphic root!

  for (size_t i = 0; i < variable_length; i++) {
    mu_variable_type_t *v = variable[i];
    if (v->reachable[0] || v->reachable[1])
      polymorphic[polymorphic_length++] = v;
  }

  // Now, we're going to collect each type that's "semipolymorphic", or
  // polymorphic because it's both reachable from matter, and it can reach a
  // polymorphic root.
  const mu_type_t *buffer[1000];
  size_t buffer_length = 0;

  /* mu_variable_type_t *variable_type = scheme->link; */
  /* while (variable_type != NULL) { */
  /*   mu_variable_type_t *next = variable_type->scheme_next; */

  /*   /1* variable_type->scheme_next = scheme->parent->link; *1/ */
  /*   /1* scheme->parent->link = variable_type; *1/ */
  /*   /1* variable_type->rank--; *1/ */

  /*   variable_type = next; */
  /* } */

  if (polymorphic_length == 0)
    return matter;

  mu_scheme_type_t *allocation;
  if ((allocation = scheme_type_allocate(induce, polymorphic_length)) == NULL)
    return NULL;

  size_t i = 0;
  for (size_t j = 0; j < polymorphic_length; j++) {
    mu_variable_type_t *type = polymorphic[j];
    type->scheme = allocation;
    type->rank = 0;
    allocation->argv[i++] = type;
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
        allocation->argv[i] = instantiate_single_type(induce, core_type->argv[i], scheme, cache, cache_i);
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
      if (variable_type->scheme != scheme) {
        cache[(*cache_i)++] = (cache_item) { &variable_type->as_type, &variable_type->as_type };
        return &variable_type->as_type;
      }

      const mu_variable_type_t *newvar;
      if ((newvar = mu_variable_type(induce)) == NULL)
        return NULL;
      cache[(*cache_i)++] = (cache_item) { &variable_type->as_type, &newvar->as_type };

      universe_iterator_t it;
      it = universe_iterator(&induce->universe, &variable_type->as_type, 0);
      for (const type_edge_t *edge; (edge = universe_next(&it)) != NULL;) {
        const mu_type_t *next = instantiate_single_type(induce, edge->source, scheme, cache, cache_i);

        // TODO: make this check unnecessary
        if (universe_search(&induce->universe, next, &newvar->as_type) != NULL)
          continue;
        append_edge(&induce->universe, next, &newvar->as_type);
      }

      it = universe_iterator(&induce->universe, &variable_type->as_type, 1);
      for (const type_edge_t *edge; (edge = universe_next(&it)) != NULL;) {
        const mu_type_t *next = instantiate_single_type(induce, edge->target, scheme, cache, cache_i);

        // TODO: make this check unnecessary
        if (universe_search(&induce->universe, &newvar->as_type, next) != NULL)
          continue;
        append_edge(&induce->universe, &newvar->as_type, next);
      }

      return &newvar->as_type;

    case IS_KIND_OF(scheme_type): {
      const mu_type_t *matter;
      if ((matter = instantiate_single_type(induce, scheme_type->matter, scheme, cache, cache_i)) == NULL)
        return NULL;

      if (matter == scheme_type->matter) {
        cache[(*cache_i)++] = (cache_item) { &scheme_type->as_type, &scheme_type->as_type };
        return &scheme_type->as_type;
      }

      const mu_scheme_type_t *result;
      size_t argc = scheme_type->argc;
      const mu_variable_type_t *const *argv = scheme_type->argv;
      if ((result = mu_scheme_type(induce, matter, argc, argv)) == NULL)
        return NULL;

      cache[(*cache_i)++] = (cache_item) { &result->as_type, &result->as_type };
      return &result->as_type;
    }
  }
  __builtin_unreachable();
}

static const mu_type_t *instantiate_scheme(
    induce_t *induce, const mu_scheme_type_t *scheme
) {
  cache_item cache[100] = {0};
  size_t i = 0;
  return instantiate_single_type(induce, scheme->matter, scheme, cache, &i);
}



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
  if ((allocation = variance_coercion_allocate(core)) == NULL)
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



induce_t *induce_initialize(
    induce_t *induce, mu_engine_t *engine, const detect_t *detect) {
  assert(detect_result(detect)->engine == engine);

  size_t node_length = engine->node_number;

  const mu_type_t **node_to_type;
  if ((node_to_type = malloc(sizeof(const mu_type_t *[node_length]))) == NULL)
    return NULL;
  for (size_t i = 0; i < node_length; node_to_type[i++] = NULL);

  node_coercion_t *node_coercion;
  if ((node_coercion = malloc(sizeof(node_coercion_t[node_length]))) == NULL)
    return NULL;
  for (size_t i = 0; i < node_length; i++)
    node_coercion[i] = (node_coercion_t) {0};

  universe_t universe;
  if (rare(universe_initialize(&universe) == NULL))
    return NULL;

  mu_id_coercion_t *id_coercion;
  if ((id_coercion = malloc(sizeof(mu_id_coercion_t))) == NULL)
    return NULL;
  *id_coercion = (mu_id_coercion_t) { .as_coercion.kind = MU_ID_COERCION };

  mu_slot_coercion_t *slot_coercion;
  if ((slot_coercion = malloc(sizeof(mu_slot_coercion_t))) == NULL)
    return NULL;
  *slot_coercion = (mu_slot_coercion_t) { .as_coercion.kind = MU_SLOT_COERCION };

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
    .node_coercion = node_coercion,
    .universe = universe,

    .id_coercion = &id_coercion->as_coercion,
    .slot_coercion = &slot_coercion->as_coercion,

    .boolean_core = boolean_core,
    .integer_core = integer_core,
    .lambda_core = lambda_core,
    .vector_core = vector_core,
  };
  return induce;
}
