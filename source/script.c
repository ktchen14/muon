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

  for (size_t i = 0; i < argc; i++)
    script->argv[i] = argv[i];

  return script;
}

const mu_sequence_expr_t *mu_script_to_sequence_expr(
    mu_engine_t *engine, const mu_script_t *script) {
  return mu_sequence_expr(engine, script->argc, script->argv);
}

const mu_sequence_expr_t *mu_script_to_sequence_expr_with_prefix(
    mu_engine_t *engine,
    const mu_script_t *script,
    size_t length,
    const mu_stmt_t *prefix[]) {
  size_t argc = script->argc + length;

  mu_sequence_expr_t *allocation;
  if ((allocation = sequence_expr_allocate(engine, argc)) == NULL)
    return NULL;

  size_t i;
  for (i = 0; i < length; i++)
    allocation->argv[i] = prefix[i];
  for (size_t j = 0; j < script->argc; j++)
    allocation->argv[i++] = script->argv[j];

  return sequence_expr_activate(allocation);
}

#include "stator/debug.h"

void mu_script_debug(const mu_script_t *script) {
  debug("Script:\n");

  WITH_DEBUG_INDENT() {
    for (size_t i = 0; i < script->argc; i++)
      mu_node_debug(&script->argv[i]->as_node);
  }
}
