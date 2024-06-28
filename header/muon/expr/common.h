#ifndef MU_EXPR_COMMON_H
#define MU_EXPR_COMMON_H

#include "../node/common.h"  // IWYU pragma: export

/**
 * @brief An enumeration of each kind of expr
 *
 * MU_ACCESS_EXPR = MU_ACCESS_EXPR_NODE,
 * MU_INTEGER_EXPR = MU_INTEGER_EXPR_NODE,
 * ...
 * MU_ZERO_EXPR = MU_ZERO_EXPR_NODE,
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

#endif /* MU_EXPR_COMMON_H */
