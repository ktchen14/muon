#ifndef MU_STATOR_EXPR_H
#define MU_STATOR_EXPR_H

#include "abstract_node.h"

#include "name.h"
#include "variable_view.h"

#include <stdint.h>

/**
 * @brief An enumeration over each kind of expr
 *
 * This will define:
 *
 * @verbatim
 *   MU_ACCESS_EXPR = MU_ACCESS_EXPR_NODE,
 *   MU_BOOLEAN_EXPR = MU_BOOLEAN_EXPR_NODE,
 *   ...
 *   MU_ZERO_EXPR = MU_ZERO_EXPR_NODE,
 * @endverbatim
 */
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_EXPR = MU_##upper##_EXPR_NODE,
  MU_EACH_EXPR_KIND(MU_EMIT)
#undef MU_EMIT
} mu_expr_kind_t;

/// An abstract expr
typedef struct {
  union {
    mu_expr_kind_t kind;
    mu_node_t as_node;
    mu_stator_t as_stator;
  };
} mu_expr_t;

/// The header that each concrete expr must have
#define MU_EXPR_HEADER union { \
  mu_expr_t as_expr; \
  mu_node_t as_node; \
  mu_stator_t as_stator; \
}

typedef struct {
  MU_EXPR_HEADER;
  const mu_name_t *name;
  const mu_expr_t *matter;
} mu_access_expr_t;

typedef struct {
  MU_EXPR_HEADER;
  _Bool data;
} mu_boolean_expr_t;

typedef struct {
  MU_EXPR_HEADER;
  const mu_expr_t *matter;
} mu_coerce_expr_t;

typedef struct {
  MU_EXPR_HEADER;
  uint64_t data;
} mu_integer_expr_t;

typedef struct {
  MU_EXPR_HEADER;
  const mu_expr_t *lambda;
  const mu_expr_t *matter;
} mu_invoke_expr_t;

typedef struct {
  MU_EXPR_HEADER;
  const mu_variable_view_t *argument;
  const mu_expr_t *matter;
} mu_lambda_expr_t;

typedef struct {
  MU_EXPR_HEADER;
  const mu_name_t *name;
} mu_name_expr_t;

typedef struct {
  const mu_name_t *name; // optional
  const mu_expr_t *expr;
} mu_expr_member_t;

typedef struct {
  MU_EXPR_HEADER;
  size_t argc;
  mu_expr_member_t argv[/* argc */];
} mu_record_expr_t;

typedef struct {
  MU_EXPR_HEADER;
  size_t argc;
  const mu_stmt_t *argv[/* argc */];
} mu_sequence_expr_t;

typedef struct {
  MU_EXPR_HEADER;
  size_t argc;
  const mu_expr_t *argv[/* argc */];
} mu_vector_expr_t;

typedef struct {
  MU_EXPR_HEADER;
} mu_zero_expr_t;

const mu_access_expr_t *mu_access_expr(
    mu_engine_t *engine, const mu_name_t *name, const mu_expr_t *matter)
  __attribute__((malloc, nonnull));

const mu_boolean_expr_t *mu_boolean_expr(mu_engine_t *engine, _Bool data)
  __attribute__((malloc, nonnull));

const mu_coerce_expr_t *mu_coerce_expr(
    mu_engine_t *engine, const mu_expr_t *matter)
  __attribute__((malloc, nonnull));

const mu_integer_expr_t *mu_integer_expr(mu_engine_t *engine, uint64_t data)
  __attribute__((malloc, nonnull));

const mu_invoke_expr_t *mu_invoke_expr(
    mu_engine_t *engine, const mu_expr_t *lambda, const mu_expr_t *matter)
  __attribute__((malloc, nonnull));

const mu_lambda_expr_t *mu_lambda_expr(
    mu_engine_t *engine, const mu_variable_view_t *argument, const mu_expr_t *matter)
  __attribute__((malloc, nonnull));

const mu_name_expr_t *mu_name_expr(mu_engine_t *engine, const mu_name_t *name)
  __attribute__((malloc, nonnull));

const mu_record_expr_t *mu_record_expr(
    mu_engine_t *engine, size_t argc, const mu_expr_member_t argv[/* argc */])
  __attribute__((malloc, nonnull(1)));

const mu_sequence_expr_t *mu_sequence_expr(
    mu_engine_t *engine, size_t argc, const mu_stmt_t *const argv[/* argc */])
  __attribute__((malloc, nonnull(1)));

const mu_vector_expr_t *mu_vector_expr(
    mu_engine_t *engine, size_t argc, const mu_expr_t *const argv[/* argc */])
  __attribute__((malloc, nonnull(1)));

const mu_zero_expr_t *mu_zero_expr(mu_engine_t *engine)
  __attribute__((malloc, nonnull));

/// Emit debugging information on the access @a expr to the debug stream
void mu_access_expr_debug(const mu_access_expr_t *expr)
  __attribute__((nonnull));

/// Emit debugging information on the boolean @a expr to the debug stream
void mu_boolean_expr_debug(const mu_boolean_expr_t *expr)
  __attribute__((nonnull));

/// Emit debugging information on the coerce @a expr to the debug stream
void mu_coerce_expr_debug(const mu_coerce_expr_t *expr)
  __attribute__((nonnull));

/// Emit debugging information on the integer @a expr to the debug stream
void mu_integer_expr_debug(const mu_integer_expr_t *expr)
  __attribute__((nonnull));

/// Emit debugging information on the invoke @a expr to the debug stream
void mu_invoke_expr_debug(const mu_invoke_expr_t *expr)
  __attribute__((nonnull));

/// Emit debugging information on the lambda @a expr to the debug stream
void mu_lambda_expr_debug(const mu_lambda_expr_t *expr)
  __attribute__((nonnull));

/// Emit debugging information on the name @a expr to the debug stream
void mu_name_expr_debug(const mu_name_expr_t *expr)
  __attribute__((nonnull));

/// Emit debugging information on the record @a expr to the debug stream
void mu_record_expr_debug(const mu_record_expr_t *expr)
  __attribute__((nonnull));

/// Emit debugging information on the sequence @a expr to the debug stream
void mu_sequence_expr_debug(const mu_sequence_expr_t *expr)
  __attribute__((nonnull));

/// Emit debugging information on the vector @a expr to the debug stream
void mu_vector_expr_debug(const mu_vector_expr_t *expr)
  __attribute__((nonnull));

/// Emit debugging information on the zero @a expr to the debug stream
void mu_zero_expr_debug(const mu_zero_expr_t *expr)
  __attribute__((nonnull));

#endif /* MU_STATOR_EXPR_H */
