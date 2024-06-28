#ifndef MU_NODE_COMMON_H
#define MU_NODE_COMMON_H

#include "../stator.h"  // IWYU pragma: export
#include "../status.h"

#include <stddef.h>

/// An enumeration of each kind of node
typedef enum {
  // Expr
  MU_ACCESS_EXPR_NODE = MU_ACCESS_EXPR_STATOR,
  MU_INTEGER_EXPR_NODE = MU_INTEGER_EXPR_STATOR,
  MU_MEMBER_EXPR_NODE = MU_MEMBER_EXPR_STATOR,
  MU_NAME_EXPR_NODE = MU_NAME_EXPR_STATOR,
  MU_RECORD_EXPR_NODE = MU_RECORD_EXPR_STATOR,
  MU_VECTOR_EXPR_NODE = MU_VECTOR_EXPR_STATOR,
  MU_ZERO_EXPR_NODE = MU_ZERO_EXPR_STATOR,

  // Sign
  MU_INTEGER_SIGN_NODE = MU_INTEGER_SIGN_STATOR,
  MU_MEMBER_SIGN_NODE = MU_MEMBER_SIGN_STATOR,
  MU_NAME_SIGN_NODE = MU_NAME_SIGN_STATOR,
  MU_RECORD_SIGN_NODE = MU_RECORD_SIGN_STATOR,
  MU_VARIABLE_SIGN_NODE = MU_VARIABLE_SIGN_STATOR,
  MU_VECTOR_SIGN_NODE = MU_VECTOR_SIGN_STATOR,

  // Stmt
  MU_CONSTANT_STMT_NODE = MU_CONSTANT_STMT_STATOR,
  MU_TYPE_STMT_NODE = MU_TYPE_STMT_STATOR,
} mu_node_kind_t;

/// An abstract node
typedef struct {
  union {
    mu_node_kind_t kind;
    mu_stator_t as_stator;
  };

  mu_source_t source;
} mu_node_t;

#endif /* MU_NODE_COMMON_H */
