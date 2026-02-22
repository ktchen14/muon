#include "induce.h"

#include "../common.h"
#include "../engine.h"
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

static MuonType *instantiate_scheme(induce_t *induce, MuonSchemeType *scheme)
  MUON_HINT_SUFFIX(nonnull);

static MuonCoercion *retrieve_core_coercion(
    induce_t *induce, MuonCoreType *source, MuonCoreType *target);

MuonCoercion *ensure_coercion(
    induce_t *induce, MuonType *source, MuonType *target) {
  // If source is the same type as target, then just return the id coercion
  if (source == target)
    return induce->id_coercion;

  // If ∃⟨source ⇒ target⟩, then just return the coercion on that edge
  type_edge_t *result_edge;
  if ((result_edge = universe_search(&induce->universe, source, target))
      != NULL)
    return coerce_with(induce, result_edge);

  // A join type shouldn't ever appear as a target
  assert(target->kind != MU_JOIN_TYPE);

  // Make ⟨source ⇒ target⟩ here in case of recursion
  if ((result_edge = append_edge(&induce->universe, source, target)) == NULL)
    return NULL;

  if (source->kind == MU_VARIABLE_TYPE && target->kind != MU_VARIABLE_TYPE) {
    universe_iterator_t it;

    // ∀(τ) | ∃⟨τ ⇒ source⟩ and τ isn't a variable type, ensure that we're able
    // to coerce τ ⇝ target
    it = universe_iterator(&induce->universe, source, 0);
    for (const type_edge_t *edge; (edge = universe_next(&it)) != NULL;) {
      MuonType *t;
      if (edge->indirect || (t = edge->source)->kind == MU_VARIABLE_TYPE)
        continue;

      if (ensure_coercion(induce, t, target) == NULL)
        return NULL;
    }

    // ∀(τ) | ∃⟨τ ⇒ source⟩ and τ is a variable type, create ⟨τ ⇒ target⟩ to
    // maintain the target-side transitive closure of τ
    it = universe_iterator(&induce->universe, source, 0);
    for (const type_edge_t *edge; (edge = universe_next(&it)) != NULL;) {
      MuonType *t;
      if (edge->indirect || (t = edge->source)->kind != MU_VARIABLE_TYPE)
        continue;

      type_edge_t *addition;
      if ((addition = edge_define(&induce->universe, t, target)) == NULL)
        return NULL;
      addition->transitive = 1;
    }

    return coerce_with(induce, result_edge);
  }

  if (source->kind != MU_VARIABLE_TYPE && target->kind == MU_VARIABLE_TYPE) {
    universe_iterator_t it;

    // ∀(τ) | ∃⟨target ⇒ τ⟩ and τ isn't a variable type, ensure that we're able
    // to coerce source ⇝ τ
    it = universe_iterator(&induce->universe, target, 1);
    for (const type_edge_t *edge; (edge = universe_next(&it)) != NULL;) {
      MuonType *t;
      if (edge->indirect || (t = edge->target)->kind == MU_VARIABLE_TYPE)
        continue;

      if (ensure_coercion(induce, source, t) == NULL)
        return NULL;
    }

    // ∀(τ) | ∃⟨target ⇒ τ⟩ and τ is a variable type, create ⟨source ⇒ τ⟩ to
    // maintain the source-side transitive closure of τ
    it = universe_iterator(&induce->universe, target, 1);
    for (const type_edge_t *edge; (edge = universe_next(&it)) != NULL;) {
      MuonType *t;
      if (edge->indirect || (t = edge->target)->kind != MU_VARIABLE_TYPE)
        continue;

      type_edge_t *addition;
      if ((addition = edge_define(&induce->universe, source, t)) == NULL)
        return NULL;
      addition->transitive = 1;
    }

    return coerce_with(induce, result_edge);
  }

  if (source->kind == MU_VARIABLE_TYPE && target->kind == MU_VARIABLE_TYPE) {
    universe_iterator_t it, jt;

    // ∀(α) | ∃⟨α ⇒ source⟩ and α isn't a variable type, ∀(β) | ∃⟨target ⇒ β⟩
    // and β isn't a variable type, ensure that we're able to coerce α ⇝ β
    it = universe_iterator(&induce->universe, source, 0);
    for (const type_edge_t *a_edge; (a_edge = universe_next(&it)) != NULL;) {
      MuonType *a;
      if (a_edge->indirect || (a = a_edge->source)->kind == MU_VARIABLE_TYPE)
        continue;

      jt = universe_iterator(&induce->universe, target, 1);
      for (const type_edge_t *b_edge; (b_edge = universe_next(&jt)) != NULL;) {
        MuonType *b;
        if (b_edge->indirect || (b = b_edge->target)->kind == MU_VARIABLE_TYPE)
          continue;

        if (ensure_coercion(induce, a, b) == NULL)
          return NULL;
      }
    }

    // ∀(α) | ∃⟨α ⇒ source⟩, create ⟨α ⇒ target⟩ to maintain the target-side
    // transitive closure of α
    it = universe_iterator(&induce->universe, source, 0);
    for (const type_edge_t *edge; (edge = universe_next(&it)) != NULL;) {
      if (edge->indirect)
        continue;

      MuonType *a = edge->source;
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

      MuonType *b = edge->target;
      type_edge_t *next_edge;
      if ((next_edge = edge_define(&induce->universe, source, b)) == NULL)
        return NULL;
      next_edge->transitive = 1;
    }

    return coerce_with(induce, result_edge);
  }

  MuonJoinType *join_type;
  if ((join_type = mu_type_cast(source, join_type)) != NULL) {
    struct MuonUnjoinCoercion *allocation;
    if ((allocation = unjoin_coercion_allocate(induce, join_type->argc))
        == NULL)
      return NULL;

    for (size_t i = 0; i < join_type->argc; i++) {
      MuonType *argument = join_type->argv[i];

      MuonCoercion *coercion;
      if ((coercion = ensure_coercion(induce, argument, target)) == NULL)
        return NULL;
      if (coercion == MU_NO_SUCH_COERCION)
        return MU_NO_SUCH_COERCION;

      allocation->argv[i] = coercion;
    }

    MuonUnjoinCoercion *result;
    if ((result = unjoin_coercion_activate(allocation, target)) == NULL)
      return NULL;
    return edge_assign(result_edge, &result->as_coercion);
  }

  // We can't handle this at this time
  assert(target->kind != MU_SCHEME_TYPE);

  MuonSchemeType *scheme_type;
  if ((scheme_type = mu_type_cast(source, scheme_type)) != NULL) {
    // Create an unscheme coercion source ⇝ instance to instantiate the scheme
    // type
    MuonType *instance;
    if ((instance = instantiate_scheme(induce, scheme_type)) == NULL)
      return NULL;

    MuonUnschemeCoercion *head;
    if ((head = mu_unscheme_coercion(induce, instance)) == NULL)
      return NULL;

    type_edge_t *edge;
    if ((edge = append_edge(&induce->universe, source, instance)) == NULL)
      return NULL;
    edge->coercion = &head->as_coercion;

    // Ensure that we're able to coerce instance ⇝ target
    MuonCoercion *coercion;
    if ((coercion = ensure_coercion(induce, instance, target)) == NULL)
      return NULL;
    if (coercion == MU_NO_SUCH_COERCION)
      return MU_NO_SUCH_COERCION;

    // Then return the coercion (source ⇝ instance) ∘ (instance ⇝ target)
    MuonIndirectCoercion *result;
    if ((result = mu_indirect_coercion(induce, &head->as_coercion, coercion))
        == NULL)
      return NULL;
    return edge_assign(result_edge, &result->as_coercion);
  }

  MuonCoreType *core_source = mu_type_cast(source, core_source);
  assert(core_source != NULL);

  MuonCoreType *core_target = mu_type_cast(target, core_target);
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
    MuonInstanceCoercion *result;
    if ((result = mu_instance_coercion(induce, target, instance)) == NULL)
      return NULL;
    return edge_assign(result_edge, &result->as_coercion);
  }

  const mu_core_t *core = source_core;

  struct MuonVarianceCoercion *allocation;
  if ((allocation = variance_coercion_allocate(induce, core)) == NULL)
    return NULL;

  for (size_t i = 0; i < core->argc; i++) {
    MuonType *next_source = core_source->argv[i];
    MuonType *next_target = core_target->argv[i];

    MuonVariance variance = core->argv[i].variance;
    assert(variance != MUON_INVARIANCE);
    if (variance == MUON_CONTRAVARIANCE) {
      MuonType *t;
      t = next_source;
      next_source = next_target;
      next_target = t;
    }

    MuonCoercion *coercion;
    if ((coercion = ensure_coercion(induce, next_source, next_target)) == NULL)
      return NULL;
    allocation->argv[i] = coercion;
  }

  MuonVarianceCoercion *result;
  if ((result = variance_coercion_activate(allocation, target)) == NULL)
    return NULL;
  return edge_assign(result_edge, &result->as_coercion);
}

MuonType *generalize_type(induce_t *induce, MuonType *root) {
  // The root type can't be polymorphic unless it's in the active scheme
  if (root->id < induce->scheme->id)
    return root;

  // Mark each type in the active scheme with whether it's accessible from the
  // root type.
  MuonType *accessible[1000] = {root};
  size_t accessible_length = 1;
  size_t polymorphic_length = 0;
  type_header(root)->access[0] = 1;

  for (MuonType *type = root; type != NULL; type = type_return(type)) {
    MuonType *next;
    _Bool next_charge;
    while ((next = type_next(type, &next_charge)) != NULL) {
      if (next->id < induce->scheme->id)
        continue;

      // Don't continue into a type that we've already accessed
      if (type_header(next)->access[next_charge])
        continue;

      if (!type_header(next)->access[0] && !type_header(next)->access[1])
        accessible[accessible_length++] = next;
      type_header(next)->access[next_charge] = 1;

      // Mark a variable type that's both + and - accessible as polymorphic
      if (next->kind == MU_VARIABLE_TYPE && type_header(next)->access[0]
          && type_header(next)->access[1]) {
        polymorphic_length++;
        type_header(next)->polymorphic = 1;
      }

      // Don't continue into a variable type from a variable type. We maintain
      // the transitive closure of each variable type, so no new information is
      // available in the next variable.
      if (next->kind == MU_VARIABLE_TYPE && type->kind == MU_VARIABLE_TYPE)
        continue;

      type = type_continue(type, next, next_charge);
    }
  };

  // If we don't have any polymorphic variables, then just reset everything and
  // return matter
  if (polymorphic_length == 0) {
    for (size_t i = 0; i < accessible_length; i++)
      type_header(accessible[i])->status = 0;
    return root;
  }

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

  for (size_t i = 0; i < accessible_length; i++)
    type_header(accessible[i])->access[0] =
        type_header(accessible[i])->access[1] = 0;
  type_header(root)->access[0] = 1;

  for (MuonType *type = root, *next;;) {
    _Bool next_charge;
    while ((next = type_next(type, &next_charge)) != NULL) {
      if (next->id < induce->scheme->id)
        continue;

      if (type_header(next)->polymorphic)
        goto handle_polymorphic;

      if (type_header(next)->access[next_charge])
        continue;
      type_header(next)->access[next_charge] = 1;
      type = type_continue(type, next, next_charge);
    }

    if ((type = type_return(next = type)) == NULL)
      break;

    if (!type_header(next)->polymorphic)
      continue;

  handle_polymorphic:
    if (!type_header(type)->polymorphic)
      polymorphic_length++;
    type_header(type)->polymorphic = 1;

    if (type->kind != MU_VARIABLE_TYPE)
      continue;

    universe_iterator_t jt;
    jt = universe_iterator(&induce->universe, type, charge);
    for (type_edge_t *edge; (edge = universe_next(&jt)) != NULL;) {
      MuonType *vertex = edge->vertex[charge];

      if (vertex == next)
        continue;
      if (type_header(vertex)->polymorphic)
        continue;
      if (vertex->kind != MU_VARIABLE_TYPE)
        continue;

      // If vertex is a sibling of next, and ∃
      if (charge == 0) {
        if (!universe_search(&induce->universe, vertex, next))
          continue;
      } else {
        if (!universe_search(&induce->universe, next, vertex))
          continue;
      }

      polymorphic_length++;
      type_header(vertex)->polymorphic = 1;
    }
  }

  assert(type_header(root)->polymorphic);

  struct MuonSchemeType *allocation;
  if ((allocation = scheme_type_allocate(induce, polymorphic_length)) == NULL)
    return NULL;

  size_t j = 0;
  for (size_t i = 0; i < accessible_length; i++) {
    MuonType *type = accessible[i];

    if (type_header(type)->polymorphic) {
      allocation->argv[j++] = type;

      MuonVariableType *v;
      if ((v = mu_type_cast(type, v)) != NULL) {
        if (v->scheme == NULL)
          ((struct MuonVariableType *) v)->scheme = allocation;
      }
    }

    type_header(type)->status = 0;
  }

  MuonSchemeType *result;
  if (rare((result = scheme_type_activate(allocation, root)) == NULL))
    return NULL;
  return &result->as_type;
}

static MuonType *instantiate_scheme(induce_t *induce, MuonSchemeType *scheme) {
  size_t length = induce->type_number;
  struct MuonType **equation;
  if ((equation = malloc(sizeof(struct MuonType *[length]))) == NULL)
    return NULL;
  for (size_t i = 0; i < length; equation[i++] = NULL)
    ;

  for (size_t i = 0; i < scheme->argc; i++) {
    switch ON_ABSTRACT_OBJECT(scheme->argv[i]) {
      case IS_CONCRETE_TYPE(MuonCoreType * nominate(core_type)) {
        struct MuonCoreType *allocation;
        if ((allocation = core_type_allocate(induce, core_type->core)) == NULL)
          return NULL;
        equation[core_type->as_type.id] = &allocation->as_type;
        break;
      }

      case IS_CONCRETE_TYPE(MuonSchemeType * nominate(scheme_type)) {
        struct MuonSchemeType *allocation;
        if ((allocation = scheme_type_allocate(induce, scheme_type->argc))
            == NULL)
          return NULL;
        equation[scheme_type->as_type.id] = &allocation->as_type;
        break;
      }

      case IS_CONCRETE_TYPE(MuonJoinType * nominate(join_type)) {
        struct MuonJoinType *allocation;
        if ((allocation = join_type_allocate(induce, join_type->argc)) == NULL)
          return NULL;
        equation[join_type->as_type.id] = &allocation->as_type;
        break;
      }

      case IS_CONCRETE_TYPE(MuonVariableType * nominate(variable_type)) {
        MuonVariableType *result;
        if ((result = mu_variable_type(induce)) == NULL)
          return NULL;
        equation[variable_type->as_type.id] =
            (struct MuonType *) &result->as_type;
        break;
      }
    }
  }

  for (size_t i = 0; i < scheme->argc; i++) {
    switch ON_ABSTRACT_OBJECT(scheme->argv[i]) {
      case IS_CONCRETE_TYPE(MuonCoreType * nominate(core_type)) {
        struct MuonCoreType *allocation =
            (struct MuonCoreType *) equation[core_type->as_type.id];
        assert(allocation != NULL);
        assert(allocation->core == core_type->core);

        for (size_t i = 0; i < core_type->core->argc; i++) {
          MuonType *type = core_type->argv[i];
          type = equation[type->id] == NULL ? type : equation[type->id];
          allocation->argv[i] = type;
        }

        if (core_type_activate(allocation) == NULL)
          return NULL;
        break;
      }

      case IS_CONCRETE_TYPE(MuonSchemeType * nominate(scheme_type)) {
        struct MuonSchemeType *allocation =
            (struct MuonSchemeType *) equation[scheme_type->as_type.id];
        assert(allocation != NULL);

        for (size_t i = 0; i < scheme_type->argc; i++) {
          MuonType *type = scheme_type->argv[i];
          type = equation[type->id] == NULL ? type : equation[type->id];
          allocation->argv[i] = type;
        }

        MuonType *matter = scheme_type->matter;
        if (equation[matter->id] != NULL)
          matter = equation[matter->id];
        if (scheme_type_activate(allocation, matter) == NULL)
          return NULL;
        break;
      }

      case IS_CONCRETE_TYPE(MuonJoinType * nominate(join_type)) {
        struct MuonJoinType *allocation =
            (struct MuonJoinType *) equation[join_type->as_type.id];
        assert(allocation != NULL);

        for (size_t i = 0; i < join_type->argc; i++) {
          MuonType *type = join_type->argv[i];
          type = equation[type->id] == NULL ? type : equation[type->id];
          allocation->argv[i] = type;
        }

        if (join_type_activate(allocation) == NULL)
          return NULL;
        break;
      }

      case IS_CONCRETE_TYPE(MuonVariableType * nominate(variable_type)) {
        struct MuonVariableType *result =
            (struct MuonVariableType *) equation[variable_type->as_type.id];
        assert(result != NULL);

        universe_iterator_t it;
        it = universe_iterator(&induce->universe, &variable_type->as_type, 0);
        for (const type_edge_t *edge; (edge = universe_next(&it)) != NULL;) {
          MuonType *source = edge->source;

          if (equation[source->id] != NULL)
            source = equation[source->id];

          // TODO: make this check unnecessary
          if (universe_search(&induce->universe, source, &result->as_type)
              != NULL)
            continue;
          append_edge(&induce->universe, source, &result->as_type);
        }

        it = universe_iterator(&induce->universe, &variable_type->as_type, 1);
        for (const type_edge_t *edge; (edge = universe_next(&it)) != NULL;) {
          MuonType *target = edge->target;

          if (equation[target->id] != NULL)
            target = equation[target->id];

          // TODO: make this check unnecessary
          if (universe_search(&induce->universe, &result->as_type, target)
              != NULL)
            continue;
          append_edge(&induce->universe, &result->as_type, target);
        }

        break;
      }
    }
  }

  MuonType *result = equation[scheme->matter->id];
  assert(result != NULL);
  return result;
}

static MuonCoercion *retrieve_core_coercion(
    induce_t *induce, MuonCoreType *source, MuonCoreType *target) {
  const mu_core_t *source_core = source->core;
  const mu_core_t *target_core = target->core;

  /* if (source_core->kind == MU_INTEGER_CORE && target_core->kind ==
   * MU_RECORD_CORE) */
  /*   return induce->id_coercion; */

  /* if (source_core->kind == MU_BOOLEAN_CORE && target_core->kind ==
   * MU_INTEGER_CORE) */
  /*   return induce->id_coercion; */

  if (source_core != target_core)
    return MU_NO_SUCH_COERCION;

  const mu_core_t *core = source_core;

  struct MuonVarianceCoercion *allocation;
  if ((allocation = variance_coercion_allocate(induce, core)) == NULL)
    return NULL;

  for (size_t i = 0; i < core->argc; i++) {
    MuonType *next_source = source->argv[i];
    MuonType *next_target = target->argv[i];

    MuonVariance variance = core->argv[i].variance;
    assert(variance != MUON_INVARIANCE);
    if (variance == MUON_CONTRAVARIANCE) {
      MuonType *t;
      t = next_source;
      next_source = next_target;
      next_target = t;
    }

    MuonCoercion *coercion = retrieve_coercion(
        induce, next_source, next_target);
    if (coercion == NULL || coercion == MU_NO_SUCH_COERCION) {
      free(allocation);
      return coercion;
    }
    allocation->argv[i] = coercion;
  }

  MuonVarianceCoercion *result;
  if ((result = variance_coercion_activate(allocation, &target->as_type))
      == NULL)
    return NULL;
  return &result->as_coercion;
}

MuonCoercion *retrieve_coercion(
    induce_t *induce, MuonType *source, MuonType *target) {
  assert(source->kind != MU_SCHEME_TYPE && target->kind != MU_SCHEME_TYPE);

  if (source == target)
    return induce->id_coercion;

  // If ∃⟨source ⇒ target⟩ then return the coercion on that edge
  type_edge_t *edge;
  if ((edge = universe_search(&induce->universe, source, target)) != NULL)
    return coerce_with(induce, edge);

  if (source->kind == MU_CORE_TYPE && target->kind == MU_CORE_TYPE) {
    MuonCoreType *next_source = (MuonCoreType *) source;
    MuonCoreType *next_target = (MuonCoreType *) target;

    MuonCoercion *result;
    if ((result = retrieve_core_coercion(induce, next_source, next_target))
        == NULL)
      return NULL;

    if (result == MU_NO_SUCH_COERCION)
      return result;

    type_edge_t *edge;
    if ((edge = append_edge(&induce->universe, source, target)) == NULL)
      return NULL;
    return edge_assign(edge, result);
  }

  return MU_NO_SUCH_COERCION;
}

induce_t *induce_initialize(
    induce_t *induce, MuonEngine *engine, const detect_t *detect) {
  assert(detect_result(detect)->engine == engine);

  size_t node_length = as_engine(engine)->node_number;

  induce_node_t *result;
  if ((result = malloc(sizeof(induce_node_t[node_length]))) == NULL)
    return NULL;
  for (size_t i = 0; i < node_length; i++)
    result[i] = (induce_node_t) {0};

  universe_t universe;
  if (rare(universe_initialize(&universe) == NULL))
    return NULL;

  struct MuonIdCoercion *id_coercion;
  if ((id_coercion = malloc(sizeof(MuonIdCoercion))) == NULL)
    return NULL;
  *id_coercion = (MuonIdCoercion) {.as_coercion.kind = MU_ID_COERCION};

  struct MuonSlotCoercion *slot_coercion;
  if ((slot_coercion = malloc(sizeof(MuonSlotCoercion))) == NULL)
    return NULL;
  *slot_coercion = (MuonSlotCoercion) {.as_coercion.kind = MU_SLOT_COERCION};

  mu_core_t *boolean_core;
  if ((boolean_core = malloc(sizeof(mu_core_t))) == NULL)
    return NULL;
  *boolean_core = (mu_core_t) {.kind = MU_BOOLEAN_CORE, .induce = induce};

  mu_core_t *integer_core;
  if ((integer_core = malloc(sizeof(mu_core_t))) == NULL)
    return NULL;
  *integer_core = (mu_core_t) {.kind = MU_INTEGER_CORE, .induce = induce};

  size_t size;

  mu_core_t *lambda_core;
  size = struct_size(mu_core_t, argv, 2);
  if ((lambda_core = malloc(size)) == NULL)
    return NULL;
  *lambda_core = (mu_core_t) {
    .kind = MU_LAMBDA_CORE, .induce = induce, .argc = 2
  };
  lambda_core->argv[0] = (mu_core_member_t) {.variance = MUON_CONTRAVARIANCE};
  lambda_core->argv[1] = (mu_core_member_t) {.variance = MUON_COVARIANCE};

  mu_core_t *vector_core;
  size = struct_size(mu_core_t, argv, 1);
  if ((vector_core = malloc(size)) == NULL)
    return NULL;
  *vector_core = (mu_core_t) {
    .kind = MU_VECTOR_CORE, .induce = induce, .argc = 1
  };
  vector_core->argv[0] = (mu_core_member_t) {.variance = MUON_COVARIANCE};

  *induce = (induce_t) {
    .engine = engine,
    .detect = detect_result(detect),
    .node_length = node_length,
    .result = result,
    .universe = universe,

    .id_coercion = &id_coercion->as_coercion,
    .slot_coercion = &slot_coercion->as_coercion,

    .boolean_core = boolean_core,
    .integer_core = integer_core,
    .lambda_core = lambda_core,
    .vector_core = vector_core,
  };

  id_coercion->as_coercion.id = induce->coercion_number++;
  id_coercion->as_coercion.inductor = induce;
  slot_coercion->as_coercion.id = induce->coercion_number++;
  slot_coercion->as_coercion.inductor = induce;

  return induce;
}

/**
 * @brief Coerce node to have type @a target. Assign the @a coercion to the
 * @a node
 *
 * Logically, the assigned @a coercion occurs to the value returned when the
 * node is evaluated.
 *
 * The behavior is undefined if:
 * - @a node and @a induce don't have the same @a engine
 * - @a node was created after @a induce
 * - a coercion has already been assigned to the @a node
 */
MuonCoercion *coerce_node(induce_t *induce, MuonNode *node, MuonType *target) {
  assert(node->engine == induce->engine);
  assert(target->induce == induce);

  MuonType *source = node_type(induce, node);

  MuonCoercion *coercion;
  if ((coercion = ensure_coercion(induce, source, target)) == NULL)
    return NULL;
  return induce->result[node->id].coercion = coercion;
}
