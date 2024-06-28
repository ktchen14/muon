#ifndef MU_SIGN_COMMON_H
#define MU_SIGN_COMMON_H

#include "../node/common.h"  // IWYU pragma: export

/**
 * @brief An enumeration of each kind of sign
 *
 * MU_INTEGER_SIGN = MU_INTEGER_SIGN_NODE,
 * ...
 * MU_VECTOR_SIGN = MU_VECTOR_SIGN_NODE,
 */
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_SIGN = MU_##upper##_SIGN_NODE,
  MU_EACH_SIGN_KIND(MU_EMIT)
#undef MU_EMIT
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
