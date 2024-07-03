#ifndef MU_NODE_COMMON_H
#define MU_NODE_COMMON_H

#include "../stator.h"  // IWYU pragma: export
#include "../status.h"

/**
 * @brief An enumeration over each kind of node
 *
 * This will define:
 *
 * @verbatim
 *   MU_ACCESS_EXPR_NODE = MU_ACCESS_EXPR_STATOR,
 *   ...
 *   MU_INTEGER_SIGN_NODE = MU_INTEGER_SIGN_STATOR,
 *   ...
 *   MU_TYPE_STMT_NODE = MU_TYPE_STMT_STATOR,
 * @endverbatim
 */
typedef enum {
#define MU_EMIT(l, upper, t, kind) \
    MU_##upper##_##kind##_NODE = MU_##upper##_##kind##_STATOR,
  MU_EACH_EXPR_KIND(MU_EMIT, EXPR)
  MU_EACH_SIGN_KIND(MU_EMIT, SIGN)
  MU_EACH_STMT_KIND(MU_EMIT, STMT)
#undef MU_EMIT
} mu_node_kind_t;

/**
 * @brief An enumeration over each kind of expr
 *
 * This will define:
 *
 * @verbatim
 *   MU_ACCESS_EXPR = MU_ACCESS_EXPR_NODE,
 *   MU_INTEGER_EXPR = MU_INTEGER_EXPR_NODE,
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
 *   MU_INTEGER_SIGN = MU_INTEGER_SIGN_NODE,
 *   MU_MEMBER_SIGN = MU_MEMBER_SIGN_NODE,
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
 *   MU_CONSTANT_STMT = MU_CONSTANT_STMT_NODE,
 *   MU_TYPE_STMT = MU_TYPE_STMT_NODE,
 * @endverbatim
 */
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_STMT = MU_##upper##_STMT_NODE,
  MU_EACH_STMT_KIND(MU_EMIT)
#undef MU_EMIT
} mu_stmt_kind_t;

/// An abstract node
typedef struct {
  union {
    mu_node_kind_t kind;
    mu_stator_t as_stator;
  };

  mu_source_t source;
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

#endif /* MU_NODE_COMMON_H */
