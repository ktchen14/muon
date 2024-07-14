#ifndef MU_STATOR_TYPE_H
#define MU_STATOR_TYPE_H

#include "common.h"  // IWYU pragma: export

/**
 * @brief An enumeration of each kind of type
 *
 * MU_BOOLEAN_TYPE = MU_BOOLEAN_TYPE_STATOR
 * ...
 * MU_VECTOR_TYPE = MU_VECTOR_TYPE_STATOR,
 */
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_TYPE = MU_##upper##_TYPE_STATOR,
  MU_EACH_TYPE_KIND(MU_EMIT)
#undef MU_EMIT
} mu_type_kind_t;

/// An abstract type
typedef struct {
  union {
    mu_type_kind_t kind;
    mu_stator_t as_stator;
  };
} mu_type_t;

/// The header that each concrete type must have
#define MU_TYPE_HEADER union { \
    mu_type_t as_type; \
    mu_stator_t as_stator; \
  }

#endif /* MU_STATOR_TYPE_H */
