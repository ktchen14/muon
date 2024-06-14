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

#endif /* MU_SCRIPT_H */
