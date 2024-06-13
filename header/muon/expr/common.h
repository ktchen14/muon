#ifndef MU_EXPR_COMMON_H
#define MU_EXPR_COMMON_H

#include "../node/common.h"

/// An enumeration of each kind of expr
typedef enum {
  MU_INTEGER_EXPR = MU_INTEGER_EXPR_NODE,
  MU_VECTOR_EXPR = MU_VECTOR_EXPR_NODE,
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

#endif /* MU_EXPR_COMMON_H */
