#ifndef MU_STATOR_RECORD_SIGN_H
#define MU_STATOR_RECORD_SIGN_H

#include "abstract_node.h"
#include "name.h"

#include <stddef.h>

typedef struct {
  const mu_name_t *name; // optional
  const mu_sign_t *sign;
} mu_sign_member_t;

typedef struct {
  MU_SIGN_HEADER;

  size_t argc;

  mu_sign_member_t argv[/* argc */];
} mu_record_sign_t;

const mu_record_sign_t *mu_record_sign(
    mu_engine_t *engine, size_t argc, const mu_sign_member_t argv[argc])
  __attribute__((malloc, nonnull(1)));

void mu_record_sign_debug(const mu_record_sign_t *sign)
  __attribute__((nonnull));

#endif /* MU_STATOR_RECORD_SIGN_H */
