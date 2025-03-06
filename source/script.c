#include "script.h"

#include "common.h"
#include "stator.h"

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

mu_script_t *mu_script(size_t argc, const mu_stmt_t *argv[argc]) {
  size_t size;
  if (rare((size = struct_size(mu_script_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_script_t *script;
  if ((script = malloc(size)) == NULL)
    return NULL;
  *script = (mu_script_t) { .argc = argc };

  memcpy(script->argv, argv, sizeof(const mu_stmt_t *[argc]));
  return script;
}

const mu_sequence_expr_t *mu_script_to_sequence_expr(
    mu_engine_t *engine, const mu_script_t *script) {
  return mu_sequence_expr(engine, script->argc, script->argv);
}

#include "stator/debug.h"

void mu_script_debug(const mu_script_t *script) {
  fprintf(stderr, "Script:\n");

  WITH_DEBUG_INDENT() {
    for (size_t i = 0; i < script->argc; i++)
      mu_stmt_debug(script->argv[i]);
  }
}
