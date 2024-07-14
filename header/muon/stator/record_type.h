#ifndef MU_STATOR_RECORD_TYPE_H
#define MU_STATOR_RECORD_TYPE_H

#include "abstract_type.h"

#include <stddef.h>

typedef struct {
  MU_TYPE_HEADER;

  size_t argc;

  const mu_type_t *argv[/* argc */];
} mu_record_type_t;

const mu_record_type_t *mu_record_type(
    mu_engine_t *engine, size_t argc, const mu_type_t *argv[argc])
  __attribute__((malloc, nonnull));

void mu_record_type_debug(const mu_record_type_t *type)
  __attribute__((nonnull));

#endif /* MU_STATOR_RECORD_TYPE_H */
