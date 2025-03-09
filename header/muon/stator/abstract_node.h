#ifndef MU_STATOR_ABSTRACT_NODE_H
#define MU_STATOR_ABSTRACT_NODE_H

#include "common.h"  // IWYU pragma: export

#include <stddef.h>

/// Expands to emit(lower, upper, title, ...) for each kind of node
#define MU_EACH_NODE_KIND(emit, ...) \
  MU_EACH_EXPR_KIND(MU_EACH_NODE_EMIT, _expr, _EXPR, Expr, emit, ##__VA_ARGS__) \
  MU_EACH_SIGN_KIND(MU_EACH_NODE_EMIT, _sign, _SIGN, Sign, emit, ##__VA_ARGS__) \
  MU_EACH_STMT_KIND(MU_EACH_NODE_EMIT, _stmt, _STMT, Stmt, emit, ##__VA_ARGS__) \
  MU_EACH_VIEW_KIND(MU_EACH_NODE_EMIT, _view, _VIEW, View, emit, ##__VA_ARGS__)

/// @internal Used as @c emit in MU_EACH_NODE_KIND
#define MU_EACH_NODE_EMIT(l, u, t, lsuffix, usuffix, tsuffix, emit, ...) \
  emit(l##lsuffix, u##usuffix, t##tsuffix, ##__VA_ARGS__)

/**
 * @brief An enumeration over each kind of node
 *
 * This will define:
 *
 * @verbatim
 *   MU_ACCESS_EXPR_NODE,
 *   ...
 *   MU_BOOLEAN_SIGN_NODE,
 *   ...
 *   MU_TYPE_STMT_NODE,
 * @endverbatim
 */
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_NODE,
  MU_EACH_NODE_KIND(MU_EMIT)
#undef MU_EMIT
} mu_node_kind_t;

/// An enumeration over each kind of expr, e.g. @c MU_ACCESS_EXPR
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_EXPR = MU_##upper##_EXPR_NODE,
  MU_EACH_EXPR_KIND(MU_EMIT)
#undef MU_EMIT
} mu_expr_kind_t;

/// An enumeration over each kind of sign, e.g. @c MU_BOOLEAN_SIGN
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_SIGN = MU_##upper##_SIGN_NODE,
  MU_EACH_SIGN_KIND(MU_EMIT)
#undef MU_EMIT
} mu_sign_kind_t;

/// An enumeration over each kind of stmt, e.g. @c MU_DEFINE_STMT
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_STMT = MU_##upper##_STMT_NODE,
  MU_EACH_STMT_KIND(MU_EMIT)
#undef MU_EMIT
} mu_stmt_kind_t;

/// An enumeration over each kind of view, e.g. @c MU_VARIABLE_VIEW
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_VIEW = MU_##upper##_VIEW_NODE,
  MU_EACH_VIEW_KIND(MU_EMIT)
#undef MU_EMIT
} mu_view_kind_t;

/// An abstract node
typedef struct mu_node_t {
  mu_node_kind_t kind;
  const mu_engine_t *engine;
  size_t id;
} mu_node_t;

/// An abstract expr
typedef struct {
  union { mu_expr_kind_t kind; mu_node_t as_node; };
} mu_expr_t;

/// An abstract sign
typedef struct {
  union { mu_sign_kind_t kind; mu_node_t as_node; };
} mu_sign_t;

/// An abstract stmt
typedef struct {
  union { mu_stmt_kind_t kind; mu_node_t as_node; };
} mu_stmt_t;

/// An abstract view
typedef struct {
  union { mu_view_kind_t kind; mu_node_t as_node; };
} mu_view_t;

#endif /* MU_STATOR_ABSTRACT_NODE_H */
