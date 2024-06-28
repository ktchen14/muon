#ifndef MU_NODE_COMMON_H
#define MU_NODE_COMMON_H

#include "../stator.h"  // IWYU pragma: export
#include "../status.h"

/**
 * @brief An enumeration of each kind of node
 *
 * MU_ACCESS_EXPR_NODE = MU_ACCESS_EXPR_STATOR,
 * ...
 * MU_INTEGER_SIGN_NODE = MU_INTEGER_SIGN_STATOR,
 * ...
 * MU_TYPE_STMT_NODE = MU_TYPE_STMT_STATOR,
 */
typedef enum {
#define MU_EMIT(l, upper, t, kind) \
    MU_##upper##_##kind##_NODE = MU_##upper##_##kind##_STATOR,
  MU_EACH_EXPR_KIND(MU_EMIT, EXPR)
  MU_EACH_SIGN_KIND(MU_EMIT, SIGN)
  MU_EACH_STMT_KIND(MU_EMIT, STMT)
#undef MU_EMIT
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
