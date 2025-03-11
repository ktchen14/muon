#include "induce.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define evince induce_reveal

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

    mu_record_coercion_t *allocation;
    if ((allocation = record_coercion_allocate(record_tactic->argc)) == NULL)
      return NULL;
    for (size_t i = 0; i < record_tactic->argc; i++)
      allocation->argv[i] = record_tactic->argv[i];

    const mu_record_coercion_t *result;
    if ((result = record_coercion_activate(allocation)) == NULL)
      return NULL;
    return &result->as_coercion;
  }

  const mu_simple_coercion_t *result;
  if ((result = mu_simple_coercion()) == NULL)
    return NULL;
  return &result->as_coercion;
}

const mu_type_t *coerce_to_lower(induce_t *induce, const mu_variable_type_t *type) {
  assert(type->assignment == NULL);

  // Determine the length of the join type
  size_t length = 0;
  for (size_t i = 0; i < induce->edge_length; i++) {
    induce_edge_t edge = induce->edge[i];
    if (edge.upper == &type->as_type && edge.lower->kind != MU_VARIABLE_TYPE)
      length++;
  }

  // Allocate the join type
  mu_join_type_t *allocation;
  if ((allocation = join_type_allocate(induce, length)) == NULL)
    return NULL;

  // Add each type as an argument
  size_t j = 0;
  for (size_t i = 0; i < induce->edge_length; i++) {
    induce_edge_t edge = induce->edge[i];
    if (edge.upper == &type->as_type && edge.lower->kind != MU_VARIABLE_TYPE)
      allocation->argv[j++] = edge.lower;
  }
  assert(j == length);

  // Activate the join type
  const mu_join_type_t *result;
  if (rare((result = join_type_activate(allocation)) == NULL))
    return NULL;

  ((mu_variable_type_t *) type)->assignment = &result->as_type;

  return &result->as_type;
}

__attribute__((nonnull)) static void access_expr_reduce(
    const mu_access_expr_t *expr, induce_t *induce) {
  const mu_type_t *type = induce->aux[expr->as_node.id];
  assert(type != NULL);

  const mu_type_t *matter_type = induce_reveal(induce, &expr->matter->as_node);

  const mu_coercion_t *coercion;
  if ((coercion = make_coercion(induce, matter_type, type)) == NULL)
    return;
  induce->coercion[expr->matter->as_node.id] = coercion;
}

__attribute__((nonnull)) static void boolean_expr_reduce(
    const mu_boolean_expr_t *expr, induce_t *induce) {}

__attribute__((nonnull)) static void integer_expr_reduce(
    const mu_integer_expr_t *expr, induce_t *induce) {}

__attribute__((nonnull)) static void invoke_expr_reduce(
    const mu_invoke_expr_t *expr, induce_t *induce) {
  const mu_type_t *type = induce->aux[expr->as_node.id];
  assert(type != NULL);

  const mu_type_t *operator_type = evince(induce, &expr->operator->as_node);

  const mu_coercion_t *coercion;
  if ((coercion = make_coercion(induce, operator_type, type)) == NULL)
    return;
  induce->coercion[expr->operator->as_node.id] = coercion;
}

__attribute__((nonnull)) static void lambda_expr_reduce(
    const mu_lambda_expr_t *expr, induce_t *induce) {}

__attribute__((nonnull)) static void name_expr_reduce(
    const mu_name_expr_t *expr, induce_t *induce) {}

__attribute__((nonnull)) static void native_expr_reduce(
    const mu_native_expr_t *expr, induce_t *induce) {}

__attribute__((nonnull)) static void record_expr_reduce(
    const mu_record_expr_t *expr, induce_t *induce) {}

__attribute__((nonnull)) static void sequence_expr_reduce(
    const mu_sequence_expr_t *expr, induce_t *induce) {}

__attribute__((nonnull)) static void vector_expr_reduce(
    const mu_vector_expr_t *expr, induce_t *induce) {
  const mu_type_t *type = induce_reveal(induce, &expr->as_node);

  const mu_simple_type_t *vector_type = mu_type_cast(type, vector_type);
  assert(vector_type != NULL);

  const mu_variable_type_t *matter_type = mu_type_cast(vector_type->argv[0], matter_type);
  assert(matter_type != NULL);

  const mu_type_t *result = coerce_to_lower(induce, matter_type);
  for (size_t i = 0; i < expr->argc; i++) {
    const mu_expr_t *argument = expr->argv[i];

    assert(induce->coercion[argument->as_node.id] == NULL);
    const mu_type_t *argument_type = induce_reveal(induce, &argument->as_node);

    const mu_coercion_t *coercion;
    if ((coercion = make_coercion(induce, argument_type, result)) == NULL)
      return;
    induce->coercion[argument->as_node.id] = coercion;
  }
}

__attribute__((nonnull)) static void zero_expr_reduce(
    const mu_zero_expr_t *expr, induce_t *induce) {}

__attribute__((nonnull)) void expr_reduce(
    const mu_expr_t *expr, induce_t *induce) {
  switch (expr->kind) {
#define MU_EMIT(lower, upper, t) \
    case MU_##upper##_EXPR: \
      lower##_expr_reduce((const mu_##lower##_expr_t *) expr, induce); \
      return;
    MU_EACH_EXPR_KIND(MU_EMIT)
#undef MU_EMIT
  }
  __builtin_unreachable();
}

const mu_type_t *reduce_origin_type(induce_t *induce, const mu_type_t *type, _Bool negative) {
  if (type->assignment != NULL)
    return type->assignment;

  assert(type->kind != MU_SCHEME_TYPE);
  assert(type->kind != MU_JOIN_TYPE);

  const mu_simple_type_t *simple_type;
  if ((simple_type = mu_type_cast(type, simple_type)) != NULL) {
    const mu_core_t *core = simple_type->core;
    _Bool remake = 0;

    for (size_t i = 0; i < core->argc; i++) {
      const mu_type_t *argument = simple_type->argv[i];

      assert(core->variance[i] != MU_INVARIANCE);
      if (core->variance[i] == MU_CONTRAVARIANCE)
        negative = !negative;

      if (reduce_origin_type(induce, argument, negative) == NULL)
        return NULL;
      assert(argument->assignment != NULL);

      if (argument->assignment != argument)
        remake = 1;
    }

    if (!remake)
      return ((mu_type_t *) type)->assignment = type;

    mu_simple_type_t *allocation;
    if ((allocation = simple_type_allocate(induce, core)) == NULL)
      return NULL;

    for (size_t i = 0; i < core->argc; i++)
      allocation->argv[i] = simple_type->argv[i]->assignment;

    const mu_simple_type_t *result;
    if ((result = simple_type_activate(allocation)) == NULL)
      return NULL;

    for (size_t i = 0; i < induce->edge_length; i++) {
      induce_edge_t edge = induce->edge[i];
      if (edge.lower != &simple_type->as_type)
        continue;
      const tactic_t *tactic = edge.tactic;
      if (append_edge(induce, &result->as_type, edge.upper, tactic) == NULL)
        return NULL;
    }

    for (size_t i = 0; i < induce->edge_length; i++) {
      induce_edge_t edge = induce->edge[i];
      if (edge.upper != &simple_type->as_type)
        continue;
      const tactic_t *tactic = edge.tactic;
      if (append_edge(induce, edge.lower, &result->as_type, tactic) == NULL)
        return NULL;
    }

    return ((mu_type_t *) type)->assignment = type;
  }

  const mu_record_type_t *record_type;
  if ((record_type = mu_type_cast(type, record_type)) != NULL) {
    _Bool remake = 0;

    for (size_t i = 0; i < record_type->argc; i++) {
      mu_type_member_t member = record_type->argv[i];

      if (reduce_origin_type(induce, member.type, negative) == NULL)
        return NULL;
      assert(member.type->assignment != NULL);

      if (member.type->assignment != member.type)
        remake = 1;
    }

    if (!remake)
      return ((mu_type_t *) type)->assignment = type;

    mu_record_type_t *allocation;
    if ((allocation = record_type_allocate(induce, record_type->argc)) == NULL)
      return NULL;

    for (size_t i = 0; i < record_type->argc; i++) {
      mu_type_member_t member = record_type->argv[i];
      allocation->argv[i] = (mu_type_member_t) {
        .name = member.name, .type = member.type->assignment,
      };
    }

    const mu_record_type_t *result;
    if ((result = record_type_activate(allocation)) == NULL)
      return NULL;

    for (size_t i = 0; i < induce->edge_length; i++) {
      induce_edge_t edge = induce->edge[i];
      if (edge.lower != &record_type->as_type)
        continue;
      const tactic_t *tactic = edge.tactic;
      if (append_edge(induce, &result->as_type, edge.upper, tactic) == NULL)
        return NULL;
    }

    for (size_t i = 0; i < induce->edge_length; i++) {
      induce_edge_t edge = induce->edge[i];
      if (edge.upper != &record_type->as_type)
        continue;
      const tactic_t *tactic = edge.tactic;
      if (append_edge(induce, edge.lower, &result->as_type, tactic) == NULL)
        return NULL;
    }

    return ((mu_type_t *) type)->assignment = type;
  }

  const mu_variable_type_t *variable_type;
  if ((variable_type = mu_type_cast(type, variable_type)) != NULL) {
    if (!negative) {
      // Determine the length of the join type
      size_t length = 0;
      for (size_t i = 0; i < induce->edge_length; i++) {
        induce_edge_t edge = induce->edge[i];
        if (edge.upper == &record_type->as_type && edge.lower->kind != MU_VARIABLE_TYPE) {
          length++;

          // Also, reduce the constituent variables
          if (reduce_origin_type(induce, edge.lower, negative) == NULL)
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
        if (edge.upper == &record_type->as_type && edge.lower->kind != MU_VARIABLE_TYPE)
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

      ((mu_variable_type_t *) type)->assignment = &result->as_type;

      return &result->as_type;
    } else {
      assert(!"Unimplemented reduction of negative variable");
    }
  }

  assert(0);
}
