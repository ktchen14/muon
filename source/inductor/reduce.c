#include "induce.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define evince induce_reveal

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

  // Add an edge for each constituent type to record the coercion to the join
  // type.
  j = 0;
  for (size_t i = 0; i < induce->edge_length; i++) {
    induce_edge_t edge = induce->edge[i];
    if (edge.upper == &type->as_type && edge.lower->kind != MU_VARIABLE_TYPE) {
      const mu_join_coercion_t *join_coercion;
      if ((join_coercion = mu_join_coercion(j)) == NULL)
        return NULL;
      if (append_edge(induce, edge.lower, &result->as_type, &join_coercion->as_coercion) == NULL)
        return NULL;
    }
  }

  for (size_t j = 0; j < result->argc; j++) {
    const mu_type_t *argument = result->argv[j];

    for (size_t i = 0; i < induce->edge_length; i++) {
      induce_edge_t edge = induce->edge[i];
      if (edge.lower == argument && edge.upper != &result->as_type && edge.upper->kind != MU_VARIABLE_TYPE) {
        if (append_edge(induce, &result->as_type, edge.upper, edge.coercion) == NULL)
          return NULL;
      }
    }
  }

  ((mu_variable_type_t *) type)->assignment = &result->as_type;

  return &result->as_type;
}

__attribute__((nonnull)) static void access_expr_reduce(
    const mu_access_expr_t *expr, induce_t *induce) {
  const mu_type_t *type = induce->aux[expr->as_node.id];
  assert(type != NULL);

  const mu_type_t *matter_type = induce_reveal(induce, &expr->matter->as_node);

  const induce_edge_t *edge = search_edge(induce, matter_type, type);
  assert(edge != NULL);

  induce->coercion[expr->matter->as_node.id] = edge->coercion;
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

  const induce_edge_t *edge = search_edge(induce, operator_type, type);
  assert(edge != NULL);

  induce->coercion[expr->operator->as_node.id] = edge->coercion;
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
    const induce_edge_t *edge = search_edge(induce, argument_type, result);
    induce->coercion[argument->as_node.id] = edge->coercion;
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
