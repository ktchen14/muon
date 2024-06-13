#ifndef MU_EXPR_COMMON_H
#define MU_EXPR_COMMON_H

#include "../stator.h"

#include <stddef.h>

/// An enumeration of each kind of expr
typedef enum {
  MU_OBJECT_EXPR = MU_OBJECT_EXPR_NODE,
  MU_VECTOR_EXPR = MU_VECTOR_EXPR_NODE,
} mu_expr_kind_t;

typedef struct {
  union {
    mu_expr_kind_t kind;
    mu_node_t as_node;
    mu_stator_t as_stator;
  };
} mu_expr_t;

#define MU_EXPR_HEADER union { \
    mu_expr_t as_expr; \
    mu_node_t as_node; \
    mu_stator_t as_stator; \
  }

#endif /* MU_EXPR_COMMON_H */
