#ifndef MU_NODE_COMMON_H
#define MU_NODE_COMMON_H

#include "../stator.h"
#include "../status.h"

#include <stddef.h>

/// An enumeration of each kind of node
typedef enum {
  // Type
  MU_RECORD_TYPE_NODE = MU_RECORD_TYPE_STATOR,
  MU_VECTOR_TYPE_NODE = MU_VECTOR_TYPE_STATOR,

  // Expr
  MU_INTEGER_EXPR_NODE = MU_INTEGER_EXPR_STATOR,
  MU_VECTOR_EXPR_NODE = MU_VECTOR_EXPR_STATOR,

  // Stmt
  MU_CONSTANT_STMT_NODE = MU_CONSTANT_STMT_STATOR,
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
