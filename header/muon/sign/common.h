#ifndef MU_SIGN_COMMON_H
#define MU_SIGN_COMMON_H

#include "../node/common.h"  // IWYU pragma: export

/// An enumeration of each kind of sign
typedef enum {
  MU_INTEGER_SIGN = MU_INTEGER_SIGN_NODE,
  MU_MEMBER_SIGN = MU_MEMBER_SIGN_NODE,
  MU_NAME_SIGN = MU_NAME_SIGN_NODE,
  MU_RECORD_SIGN = MU_RECORD_SIGN_NODE,
  MU_VECTOR_SIGN = MU_VECTOR_SIGN_NODE,
} mu_sign_kind_t;

/// An abstract sign
typedef struct {
  union {
    mu_sign_kind_t kind;
    mu_node_t as_node;
    mu_stator_t as_stator;
  };
} mu_sign_t;

/// The header that each concrete sign must have
#define MU_SIGN_HEADER union { \
    mu_sign_t as_sign; \
    mu_node_t as_node; \
    mu_stator_t as_stator; \
  }

#endif /* MU_SIGN_COMMON_H */
