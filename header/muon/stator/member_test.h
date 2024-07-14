#ifndef MU_STATOR_MEMBER_CONSTRAINT_H
#define MU_STATOR_MEMBER_CONSTRAINT_H

#include "abstract_test.h"  // IWYU pragma: export

#include "name.h"

typedef struct {
  MU_TEST_HEADER;

  const mu_name_t *name;
} mu_member_test_t;

const mu_member_test_t *mu_member_test(
    mu_engine_t *engine, const mu_name_t *name)
  __attribute__((malloc, nonnull));

void mu_member_test_debug(const mu_member_test_t *test)
  __attribute__((nonnull));

#endif /* MU_STATOR_MEMBER_CONSTRAINT_H */
