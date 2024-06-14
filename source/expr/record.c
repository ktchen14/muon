#include "record.h"

#include "../common.h"
#include "../engine.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>

const mu_record_expr_t *mu_record_expr(
    mu_engine_t *engine, size_t argc, const mu_expr_t *argv[argc]) {
  for (size_t i = 0; i < argc; i++)
    assert(argv[i]->as_stator.engine == engine);

  size_t size;
  if (rare((size = struct_size(mu_record_expr_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_record_expr_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_record_expr_t) {
    .as_expr.kind = MU_RECORD_EXPR, .argc = argc,
  };
  memcpy(&result->argv, argv, sizeof(const mu_expr_t *[argc]));

  return engine_assign_concrete(engine, result);
}
