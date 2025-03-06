#ifndef MU_STATOR_EXPR_I
#define MU_STATOR_EXPR_I

#include <muon/stator/expr.h>  // IWYU pragma: export

#include <stddef.h>

/// Return the <em>i</em>th node in the access @a expr
__attribute__((nonnull, pure))
static inline const mu_node_t *access_expr_at(
    const mu_access_expr_t *expr, size_t i) {
  return i == 0 ? &expr->matter->as_node : NULL;
}

/// Return the <em>i</em>th node in the boolean @a expr
__attribute__((const, nonnull))
static inline const mu_node_t *boolean_expr_at(
    const mu_boolean_expr_t *expr, size_t i) {
  return NULL;
}

/// Return the <em>i</em>th node in the coerce @a expr
__attribute__((nonnull, pure))
static inline const mu_node_t *coerce_expr_at(
    const mu_coerce_expr_t *expr, size_t i) {
  return i == 0 ? &expr->matter->as_node : NULL;
}

/// Return the <em>i</em>th node in the integer @a expr
__attribute__((const, nonnull))
static inline const mu_node_t *integer_expr_at(
    const mu_integer_expr_t *expr, size_t i) {
  return NULL;
}

/// Return the <em>i</em>th node in the invoke @a expr
__attribute__((nonnull, pure))
static inline const mu_node_t *invoke_expr_at(
    const mu_invoke_expr_t *expr, size_t i) {
  switch (i) {
    case 0: return &expr->lambda->as_node;
    case 1: return &expr->matter->as_node;
    default: return NULL;
  }
}

/// Return the <em>i</em>th node in the lambda @a expr
__attribute__((nonnull, pure))
static inline const mu_node_t *lambda_expr_at(
    const mu_lambda_expr_t *expr, size_t i) {
  switch (i) {
    case 0: return &expr->argument->as_node;
    case 1: return &expr->matter->as_node;
    default: return NULL;
  }
}

/// Return the <em>i</em>th node in the name @a expr
__attribute__((nonnull, pure))
static inline const mu_node_t *name_expr_at(
    const mu_name_expr_t *expr, size_t i) {
  return NULL;
}

/// Return the <em>i</em>th node in the record @a expr
__attribute__((nonnull, pure))
static inline const mu_node_t *record_expr_at(
    const mu_record_expr_t *expr, size_t i) {
  return i < expr->argc ? &expr->argv[i].expr->as_node : NULL;
}

/// Return the <em>i</em>th node in the sequence @a expr
__attribute__((nonnull, pure))
static inline const mu_node_t *sequence_expr_at(
    const mu_sequence_expr_t *expr, size_t i) {
  return i < expr->argc ? &expr->argv[i]->as_node : NULL;
}

/// Return the <em>i</em>th node in the vector @a expr
__attribute__((nonnull, pure))
static inline const mu_node_t *vector_expr_at(
    const mu_vector_expr_t *expr, size_t i) {
  return i < expr->argc ? &expr->argv[i]->as_node : NULL;
}

/// Return the <em>i</em>th node in the zero @a expr
__attribute__((const, nonnull))
static inline const mu_node_t *zero_expr_at(
    const mu_zero_expr_t *expr, size_t i) {
  return NULL;
}

mu_record_expr_t *record_expr_allocate(mu_engine_t *engine, size_t argc)
  __attribute__((malloc, nonnull));

const mu_record_expr_t *record_expr_activate(mu_record_expr_t *expr)
  __attribute__((nonnull));

#endif /* MU_STATOR_EXPR_I */
