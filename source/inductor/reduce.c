#include "induce.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define evince induce_reveal

const mu_type_t *reduce_type(induce_t *induce, const mu_type_t *type, _Bool negative);

const mu_coercion_t *make_coercion(
    induce_t *induce, const mu_type_t *source, const mu_type_t *target) {
  assert(source->kind != MU_JOIN_TYPE);

  if (target->kind == MU_JOIN_TYPE) {
    const mu_join_type_t *join_type = (const mu_join_type_t *) target;

    for (size_t i = 0; i < join_type->argc; i++) {
      const mu_type_t *type = join_type->argv[i];

      const mu_coercion_t *coercion;
      if (source == type) {
        coercion = &induce->id_coercion->as_coercion;
      } else {
        const induce_edge_t *edge;
        if ((edge = search_edge(induce, source, type)) == NULL)
          continue;

        if ((coercion = make_coercion(induce, source, type)) == NULL)
          return NULL;
      }

      const mu_join_coercion_t *result;
      if ((result = mu_join_coercion(i)) == NULL)
        return NULL;
      return &result->as_coercion;
    }

    assert(0);
  }

  const induce_edge_t *edge;
  edge = search_edge(induce, source, target);
  assert(edge != NULL);

  const tactic_t *tactic = edge->tactic;
  if (tactic == NULL)
    return &induce->id_coercion->as_coercion;

  if (tactic->kind == RECORD_TACTIC) {
    const record_tactic_t *record_tactic = (const record_tactic_t *) tactic;

    const mu_record_coercion_t *result;
    if ((result = mu_record_coercion(record_tactic->instance)) == NULL)
      return NULL;
    return &result->as_coercion;
  }

  const mu_simple_coercion_t *result;
  if ((result = mu_simple_coercion()) == NULL)
    return NULL;
  return &result->as_coercion;
}

__attribute__((nonnull)) static const mu_type_t *access_expr_reduce(
    const mu_access_expr_t *expr, induce_t *induce, const mu_type_t *type) {
  const mu_type_t *aux_type = induce->aux[expr->as_node.id];
  assert(aux_type != NULL);

  if ((aux_type = reduce_type(induce, aux_type, 0)) == NULL)
    return NULL;

  const mu_type_t *matter_type = evince(induce, &expr->matter->as_node);

  const mu_coercion_t *coercion;
  if ((coercion = make_coercion(induce, matter_type, aux_type)) == NULL)
    return NULL;
  induce->coercion[expr->matter->as_node.id] = coercion;

  const mu_core_type_t *core_type = mu_type_cast(aux_type, core_type);
  assert(core_type != NULL);
  assert(core_type->core->kind == MU_RECORD_CORE);
  assert(core_type->core->argc == 1);
  assert(core_type->core->argv[0].name == expr->name);

  return core_type->argv[0];
}

__attribute__((nonnull)) static const mu_type_t *boolean_expr_reduce(
    const mu_boolean_expr_t *expr, induce_t *induce, const mu_type_t *type) {
  return type;
}

__attribute__((nonnull)) static const mu_type_t *integer_expr_reduce(
    const mu_integer_expr_t *expr, induce_t *induce, const mu_type_t *type) {
  return type;
}

__attribute__((nonnull)) static const mu_type_t *invoke_expr_reduce(
    const mu_invoke_expr_t *expr, induce_t *induce, const mu_type_t *type) {
  const mu_type_t *aux_type = induce->aux[expr->as_node.id];
  assert(aux_type != NULL);

  const mu_type_t *operator_type = evince(induce, &expr->operator->as_node);

  const mu_coercion_t *coercion;
  if ((coercion = make_coercion(induce, operator_type, aux_type)) == NULL)
    return NULL;
  induce->coercion[expr->operator->as_node.id] = coercion;

  return reduce_type(induce, type, 0);
}

__attribute__((nonnull)) static const mu_type_t *lambda_expr_reduce(
    const mu_lambda_expr_t *expr, induce_t *induce, const mu_type_t *type) {
  return reduce_type(induce, type, 0);
}

__attribute__((nonnull)) static const mu_type_t *name_expr_reduce(
    const mu_name_expr_t *expr, induce_t *induce, const mu_type_t *type) {
  return reduce_type(induce, type, 0);
}

__attribute__((nonnull)) static const mu_type_t *native_expr_reduce(
    const mu_native_expr_t *expr, induce_t *induce, const mu_type_t *type) {
  return reduce_type(induce, type, 0);
}

__attribute__((nonnull)) static const mu_type_t *record_expr_reduce(
    const mu_record_expr_t *expr, induce_t *induce, const mu_type_t *type) {
  return reduce_type(induce, type, 0);
}

__attribute__((nonnull)) static const mu_type_t *sequence_expr_reduce(
    const mu_sequence_expr_t *expr, induce_t *induce, const mu_type_t *type) {
  return reduce_type(induce, type, 0);
}

__attribute__((nonnull)) static const mu_type_t *vector_expr_reduce(
    const mu_vector_expr_t *expr, induce_t *induce, const mu_type_t *type) {
  if ((type = reduce_type(induce, type, 0)) == NULL)
    return NULL;

  const mu_core_type_t *vector_type = mu_type_cast(type, vector_type);
  assert(vector_type != NULL);
  const mu_type_t *matter_type = vector_type->argv[0];

  for (size_t i = 0; i < expr->argc; i++) {
    const mu_expr_t *argument = expr->argv[i];
    const mu_type_t *argument_type = evince(induce, &argument->as_node);

    const mu_coercion_t *coercion;
    if ((coercion = make_coercion(induce, argument_type, matter_type)) == NULL)
      return NULL;
    induce->coercion[argument->as_node.id] = coercion;
  }

  return type;
}

__attribute__((nonnull)) static const mu_type_t *zero_expr_reduce(
    const mu_zero_expr_t *expr, induce_t *induce, const mu_type_t *type) {
  return reduce_type(induce, type, 0);
}

__attribute__((nonnull)) static const mu_type_t *expr_reduce(
    const mu_expr_t *expr, induce_t *induce, const mu_type_t *type) {
  switch (expr->kind) {
#define MU_EMIT(lower, upper, t) \
    case MU_##upper##_EXPR: \
      return lower##_expr_reduce((const mu_##lower##_expr_t *) expr, induce, type);
    MU_EACH_EXPR_KIND(MU_EMIT)
#undef MU_EMIT
  }
  __builtin_unreachable();
}

const mu_type_t *reduce_core_type(
    induce_t *induce, const mu_core_type_t *origin, _Bool negative) {
  const mu_core_t *core = origin->core;
  _Bool change = 0;

  for (size_t i = 0; i < core->argc; i++) {
    const mu_type_t *argument = origin->argv[i];

    _Bool argument_negative = negative;
    mu_variance_t variance = core->argv[i].variance;
    assert(variance != MU_INVARIANCE);
    if (variance == MU_CONTRAVARIANCE)
      argument_negative = !argument_negative;

    const mu_type_t *assignment;
    if ((assignment = reduce_type(induce, argument, argument_negative)) == NULL)
      return NULL;
    assert(assignment == argument->assignment);

    change |= assignment != argument;
  }

  if (!change)
    return ((mu_type_t *) origin)->assignment = &origin->as_type;

  mu_core_type_t *allocation;
  if ((allocation = core_type_allocate(induce, core)) == NULL)
    return NULL;

  for (size_t i = 0; i < core->argc; i++)
    allocation->argv[i] = origin->argv[i]->assignment;

  const mu_core_type_t *result;
  if ((result = core_type_activate(allocation)) == NULL)
    return NULL;

  // TODO: not sure if this is correct
  if (negative) {
    for (size_t i = 0; i < induce->edge_length; i++) {
      induce_edge_t edge = induce->edge[i];
      if (edge.lower != &origin->as_type)
        continue;
      if (append_edge(induce, &result->as_type, edge.upper, edge.tactic) == NULL)
        return NULL;
    }
  } else {
    for (size_t i = 0; i < induce->edge_length; i++) {
      induce_edge_t edge = induce->edge[i];
      if (edge.upper != &origin->as_type)
        continue;
      if (append_edge(induce, edge.lower, &result->as_type, edge.tactic) == NULL)
        return NULL;
    }
  }

  return ((mu_type_t *) origin)->assignment = &result->as_type;
}

const mu_type_t *reduce_type(induce_t *induce, const mu_type_t *type, _Bool negative) {
  if (type->assignment != NULL)
    return type->assignment;

  assert(type->kind != MU_SCHEME_TYPE);
  assert(type->kind != MU_JOIN_TYPE);

  const mu_core_type_t *core_type;
  if ((core_type = mu_type_cast(type, core_type)) != NULL)
    return reduce_core_type(induce, core_type, negative);

  const mu_variable_type_t *variable_type;
  if ((variable_type = mu_type_cast(type, variable_type)) != NULL) {
    if (!negative) {
      // Determine the length of the join type
      size_t length = 0;
      for (size_t i = 0; i < induce->edge_length; i++) {
        induce_edge_t edge = induce->edge[i];
        if (edge.upper == &variable_type->as_type && edge.lower->kind != MU_VARIABLE_TYPE) {
          length++;

          // Also, reduce the constituent variables
          if (reduce_type(induce, edge.lower, negative) == NULL)
            return NULL;
          assert(edge.lower->assignment != NULL);
        }
      }

      // Allocate the join type
      mu_join_type_t *allocation;
      if ((allocation = join_type_allocate(induce, length)) == NULL)
        return NULL;

      // Add each type as an argument
      size_t j = 0;
      for (size_t i = 0; i < induce->edge_length; i++) {
        induce_edge_t edge = induce->edge[i];
        if (edge.upper == &variable_type->as_type && edge.lower->kind != MU_VARIABLE_TYPE)
          allocation->argv[j++] = edge.lower;
      }
      assert(j == length);

      // Activate the join type
      const mu_join_type_t *result;
      if (rare((result = join_type_activate(allocation)) == NULL))
        return NULL;

      // TODO: fix tactics here

      // Add an edge for each constituent type to record the coercion to the join
      // type.
      j = 0;
      for (size_t i = 0; i < induce->edge_length; i++) {
        induce_edge_t edge = induce->edge[i];
        if (edge.upper == &variable_type->as_type && edge.lower->kind != MU_VARIABLE_TYPE) {
          const mu_join_coercion_t *join_coercion;
          if ((join_coercion = mu_join_coercion(j)) == NULL)
            return NULL;
          if (append_edge(induce, edge.lower, &result->as_type, NULL) == NULL)
            return NULL;
          j++;
        }
      }

      for (size_t j = 0; j < result->argc; j++) {
        const mu_type_t *argument = result->argv[j];

        for (size_t i = 0; i < induce->edge_length; i++) {
          induce_edge_t edge = induce->edge[i];
          if (edge.lower == argument && edge.upper != &result->as_type && edge.upper->kind != MU_VARIABLE_TYPE) {
            if (append_edge(induce, &result->as_type, edge.upper, NULL) == NULL)
              return NULL;
          }
        }
      }

      ((mu_variable_type_t *) variable_type)->assignment = &result->as_type;
      ((mu_type_t *) variable_type)->assignment = &result->as_type;

      return &result->as_type;
    } else {
      assert(!"Unimplemented reduction of negative variable");
    }
  }

  assert(0);
}

const mu_type_t *handle_node_reduction(induce_t *induce, const mu_node_t *root) {
  assert(root->id < induce->node_length);

  const mu_node_t *node = root, *next;
  do {
    while ((next = node_at(node, node_cursor(node)->i++)) != NULL)
      node = node_continue(node, next);

    const mu_expr_t *expr;
    if ((expr = mu_node_cast(node, expr)) == NULL)
      continue;

    const mu_type_t *type = evince(induce, &expr->as_node);
    assert(type != NULL);

    if ((type = expr_reduce(expr, induce, type)) == NULL)
      return NULL;
    induce->node_to_type[node->id] = type;
  } while ((node = node_return(node)) != NULL);

  return evince(induce, root);
}
