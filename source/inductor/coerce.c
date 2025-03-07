#include "induce.h"

#include <stdlib.h>

const mu_type_t *coerce_to_lower(induce_t *induce, const mu_variable_type_t *type);

__attribute__((nonnull)) static void access_expr_coerce(
    const mu_access_expr_t *expr, induce_t *induce) {
  const mu_type_t *type = induce->aux[expr->as_stator.id];
  assert(type != NULL);

  const mu_type_t *matter_type = induce_reveal(induce, &expr->matter->as_node);

  const induce_edge_t *edge = search_edge(induce, matter_type, type);
  assert(edge != NULL);

  induce->coercion[expr->matter->as_stator.id] = edge->coercion;
}

__attribute__((nonnull)) static void boolean_expr_coerce(
    const mu_boolean_expr_t *expr, induce_t *induce) {}

__attribute__((nonnull)) static void coerce_expr_coerce(
    const mu_coerce_expr_t *expr, induce_t *induce) {
  abort();
}

__attribute__((nonnull)) static void integer_expr_coerce(
    const mu_integer_expr_t *expr, induce_t *induce) {}

__attribute__((nonnull)) static void invoke_expr_coerce(
    const mu_invoke_expr_t *expr, induce_t *induce) {
  const mu_type_t *type = induce->aux[expr->as_stator.id];
  assert(type != NULL);
}

__attribute__((nonnull)) static void lambda_expr_coerce(
    const mu_lambda_expr_t *expr, induce_t *induce) {}

__attribute__((nonnull)) static void name_expr_coerce(
    const mu_name_expr_t *expr, induce_t *induce) {}

__attribute__((nonnull)) static void native_expr_coerce(
    const mu_native_expr_t *expr, induce_t *induce) {}

__attribute__((nonnull)) static void record_expr_coerce(
    const mu_record_expr_t *expr, induce_t *induce) {}

__attribute__((nonnull)) static void sequence_expr_coerce(
    const mu_sequence_expr_t *expr, induce_t *induce) {}

__attribute__((nonnull)) static void vector_expr_coerce(
    const mu_vector_expr_t *expr, induce_t *induce) {
  const mu_type_t *type = induce_reveal(induce, &expr->as_node);

  const mu_simple_type_t *vector_type = mu_type_cast(type, vector_type);
  assert(vector_type != NULL);

  const mu_variable_type_t *matter_type = mu_type_cast(vector_type->argv[0], matter_type);
  assert(matter_type != NULL);

  const mu_type_t *result = coerce_to_lower(induce, matter_type);
  for (size_t i = 0; i < expr->argc; i++) {
    const mu_expr_t *argument = expr->argv[i];

    assert(induce->coercion[argument->as_stator.id] == NULL);
    const mu_type_t *argument_type = induce_reveal(induce, &argument->as_node);
    const induce_edge_t *edge = search_edge(induce, argument_type, result);
    induce->coercion[argument->as_stator.id] = edge->coercion;
  }
}

__attribute__((nonnull)) static void zero_expr_coerce(
    const mu_zero_expr_t *expr, induce_t *induce) {}

__attribute__((nonnull)) static void expr_coerce(
    const mu_expr_t *expr, induce_t *induce) {
  switch (expr->kind) {
#define MU_EMIT(lower, upper, t) \
    case MU_##upper##_EXPR: \
      lower##_expr_coerce((const mu_##lower##_expr_t *) expr, induce); \
      break;
    MU_EACH_EXPR_KIND(MU_EMIT)
#undef MU_EMIT
  }
  __builtin_unreachable();
}
