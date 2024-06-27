#include "script.h"

#include "common.h"
#include "stmt.h"

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

const mu_sign_t *const *mu_script_induce(const mu_script_t *script) {
  const mu_sign_t **result;
  if ((result = malloc(sizeof(const mu_sign_t *[256]))) == NULL)
    return NULL;
  for (size_t i = 0; i < 255; i++)
    result[i] = NULL;

  for (size_t i = 0; i < script->argc; i++) {
    const mu_stmt_t *stmt = script->argv[i];
    if (stmt->kind != MU_CONSTANT_STMT)
      continue;

    const mu_constant_stmt_t *constant_stmt = (const mu_constant_stmt_t *) stmt;
    const mu_sign_t *sign = constant_stmt_induce(constant_stmt);
    result[constant_stmt->as_stator.id] = sign;
  }

  return result;
}

void mu_script_debug(const mu_script_t *script) {
  fprintf(stderr, "Script:\n");

  WITH_DEBUG_INDENT() {
    for (size_t i = 0; i < script->argc; i++)
      mu_stmt_debug(script->argv[i]);
  }
}
