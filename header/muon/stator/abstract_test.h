#ifndef MU_STATOR_ABSTRACT_TEST_H
#define MU_STATOR_ABSTRACT_TEST_H

#include "common.h"  // IWYU pragma: export

/**
 * @brief An enumeration of each kind of test
 *
 * MU_MEMBER_TEST = MU_MEMBER_TEST_STATOR
 */
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_TEST = MU_##upper##_TEST_STATOR,
  MU_EACH_TEST_KIND(MU_EMIT)
#undef MU_EMIT
} mu_test_kind_t;

/// An abstract test
typedef struct {
  union {
    mu_test_kind_t kind;
    mu_stator_t as_stator;
  };
} mu_test_t;

/// The header that each concrete test must have
#define MU_TEST_HEADER union { \
    mu_test_t as_test; \
    mu_stator_t as_stator; \
  }

#endif /* MU_STATOR_ABSTRACT_TEST_H */
