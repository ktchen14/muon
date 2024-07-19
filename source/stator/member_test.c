#include "member_test.h"

#include "engine.h"
#include "name.h"
#include "test.h"
#include "type.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

const mu_member_test_t *mu_member_test(
    mu_engine_t *engine, const mu_name_t *name, const mu_type_t *type) {
  assert(name->as_stator.engine == engine);
  assert(type->as_stator.engine == engine);

  size_t size = sizeof(mu_member_test_t);

  mu_member_test_t *result;
  if ((result = test_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_member_test_t) {
    .as_test.kind = MU_MEMBER_TEST, .name = name, .type = type,
  };

  return assign_test(engine, result);
}

void mu_member_test_debug(const mu_member_test_t *test) {
  mu_name_debug(test->name);
  fputs(": ", stderr);
  mu_type_debug(test->type);
}
