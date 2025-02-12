#ifndef MU_STATOR_ABSTRACT_NODE_H
#define MU_STATOR_ABSTRACT_NODE_H

#include "common.h"  // IWYU pragma: export

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
 *   MU_ACCESS_EXPR_NODE = MU_ACCESS_EXPR_STATOR,
 *   ...
 *   MU_BOOLEAN_SIGN_NODE = MU_BOOLEAN_SIGN_STATOR,
 *   ...
 *   MU_TYPE_STMT_NODE = MU_TYPE_STMT_STATOR,
 * @endverbatim
 */
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_NODE = MU_##upper##_STATOR,
  MU_EACH_NODE_KIND(MU_EMIT)
#undef MU_EMIT
} mu_node_kind_t;

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

/**
 * @brief An enumeration over each kind of sign
 *
 * This will define:
 *
 * @verbatim
 *   MU_BOOLEAN_SIGN = MU_BOOLEAN_SIGN_NODE,
 *   MU_INTEGER_SIGN = MU_INTEGER_SIGN_NODE,
 *   ...
 *   MU_VECTOR_SIGN = MU_VECTOR_SIGN_NODE,
 * @endverbatim
 */
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_SIGN = MU_##upper##_SIGN_NODE,
  MU_EACH_SIGN_KIND(MU_EMIT)
#undef MU_EMIT
} mu_sign_kind_t;

/**
 * @brief An enumeration over each kind of stmt
 *
 * This will define:
 *
 * @verbatim
 *   MU_DEFINE_STMT = MU_DEFINE_STMT_NODE,
 *   MU_TYPE_STMT = MU_TYPE_STMT_NODE,
 * @endverbatim
 */
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_STMT = MU_##upper##_STMT_NODE,
  MU_EACH_STMT_KIND(MU_EMIT)
#undef MU_EMIT
} mu_stmt_kind_t;

/**
 * @brief An enumeration over each kind of view
 *
 * This will define:
 *
 * @verbatim
 *   MU_VARIABLE_VIEW = MU_VARIABLE_VIEW_NODE,
 * @endverbatim
 */
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_VIEW = MU_##upper##_VIEW_NODE,
  MU_EACH_VIEW_KIND(MU_EMIT)
#undef MU_EMIT
} mu_view_kind_t;

/// An abstract node
typedef struct mu_node_t {
  union {
    mu_node_kind_t kind;
    mu_stator_t as_stator;
  };
} mu_node_t;

/// An abstract expr
typedef struct {
  union {
    mu_expr_kind_t kind;
    mu_node_t as_node;
    mu_stator_t as_stator;
  };
} mu_expr_t;

/// An abstract sign
typedef struct {
  union {
    mu_sign_kind_t kind;
    mu_node_t as_node;
    mu_stator_t as_stator;
  };
} mu_sign_t;

/// An abstract stmt
typedef struct {
  union {
    mu_stmt_kind_t kind;
    mu_node_t as_node;
    mu_stator_t as_stator;
  };
} mu_stmt_t;

/// An abstract view
typedef struct {
  union {
    mu_view_kind_t kind;
    mu_node_t as_node;
    mu_stator_t as_stator;
  };
} mu_view_t;

/// The header that each concrete node must have
#define MU_NODE_HEADER union { \
  mu_node_t as_node; \
  mu_stator_t as_stator; \
}

/// The header that each concrete expr must have
#define MU_EXPR_HEADER union { \
  mu_expr_t as_expr; \
  mu_node_t as_node; \
  mu_stator_t as_stator; \
}

/// The header that each concrete sign must have
#define MU_SIGN_HEADER union { \
  mu_sign_t as_sign; \
  mu_node_t as_node; \
  mu_stator_t as_stator; \
}

/// The header that each concrete stmt must have
#define MU_STMT_HEADER union { \
  mu_stmt_t as_stmt; \
  mu_node_t as_node; \
  mu_stator_t as_stator; \
}

/// The header that each concrete view must have
#define MU_VIEW_HEADER union { \
  mu_view_t as_view; \
  mu_node_t as_node; \
  mu_stator_t as_stator; \
}

#endif /* MU_STATOR_ABSTRACT_NODE_H */
