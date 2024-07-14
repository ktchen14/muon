#ifndef MU_STATOR_RECORD_SIGN_H
#define MU_STATOR_RECORD_SIGN_H

#include "abstract_node.h"

#include <stddef.h>

typedef struct {
  MU_SIGN_HEADER;

  size_t argc;

  const mu_sign_t *argv[/* argc */];
} mu_record_sign_t;

const mu_record_sign_t *mu_record_sign(
    mu_engine_t *engine,
    size_t argc,
    const mu_sign_t *const argv[argc],
    const mu_node_source_t *source)
  __attribute__((malloc, nonnull(1)));

void mu_record_sign_debug(const mu_record_sign_t *sign)
  __attribute__((nonnull));

#endif /* MU_STATOR_RECORD_SIGN_H */
