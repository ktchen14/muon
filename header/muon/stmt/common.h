#ifndef MU_STMT_COMMON_H
#define MU_STMT_COMMON_H

#include "../node/common.h"

/// An enumeration of each kind of stmt
typedef enum {
  MU_CONSTANT_STMT = MU_CONSTANT_STMT_NODE,
  MU_TYPE_STMT = MU_TYPE_STMT_NODE,
} mu_stmt_kind_t;

/// An abstract stmt
typedef struct {
  union {
    mu_stmt_kind_t kind;
    mu_node_t as_node;
    mu_stator_t as_stator;
  };
} mu_stmt_t;

/// The header that each concrete stmt must have
#define MU_STMT_HEADER union { \
    mu_stmt_t as_stmt; \
    mu_node_t as_node; \
    mu_stator_t as_stator; \
  }

#endif /* MU_STMT_COMMON_H */
