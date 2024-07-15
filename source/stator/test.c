#include "test.h"

void mu_test_debug(const mu_test_t *test) {
  switch (test->kind) {
#define MU_EMIT(lower, upper, t) \
    case MU_##upper##_TEST: \
      mu_##lower##_test_debug((const mu_##lower##_test_t *) test); \
      break;
  MU_EACH_TEST_KIND(MU_EMIT)
#undef MU_EMIT
  }
}
