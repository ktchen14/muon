#include "record_sign.h"

#include "../engine.h"
#include "../node.h"
#include "../status.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

const mu_record_sign_t *mu_record_sign(
    mu_engine_t *engine,
    size_t argc,
    const mu_sign_t *argv[argc],
    const mu_source_t *source) {
  for (size_t i = 0; i < argc; i++)
    assert(argv[i]->as_stator.engine == engine);

  size_t size;
  if (rare((size = struct_size(mu_record_sign_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_record_sign_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_record_sign_t) {
    .as_sign.kind = MU_RECORD_SIGN, .argc = argc,
  };
  memcpy(&result->argv, argv, sizeof(const mu_sign_t *[argc]));

  if (source != NULL)
    result->as_node.source = *source;

  return node_assign(engine, result);
}

void mu_record_sign_debug(const mu_record_sign_t *sign) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Record Sign #%zu:\n", sign->as_stator.id);

  WITH_DEBUG_INDENT() {
    for (size_t i = 0; i < sign->argc; i++)
      mu_sign_debug(sign->argv[i]);
  }
}
