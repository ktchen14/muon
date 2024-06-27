#ifndef MU_SCRIPT_H
#define MU_SCRIPT_H

#include "stmt.h"

#include <stddef.h>

typedef struct {
  size_t argc;
  const mu_stmt_t *argv[/* argc */];
} mu_script_t;

mu_script_t *mu_script(size_t argc, const mu_stmt_t *argv[argc])
  __attribute__((malloc));

#include "sign.h"

const mu_sign_t *const *mu_script_induce(const mu_script_t *script)
  __attribute__((nonnull));

void mu_script_debug(const mu_script_t *script) __attribute__((nonnull));

#endif /* MU_SCRIPT_H */
