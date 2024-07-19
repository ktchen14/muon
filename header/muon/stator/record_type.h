#ifndef MU_STATOR_RECORD_TYPE_H
#define MU_STATOR_RECORD_TYPE_H

#include "abstract_type.h"
#include "name.h"

#include <stddef.h>

typedef struct {
  const mu_name_t *name; // optional
  const mu_type_t *type;
} mu_type_member_t;

typedef struct {
  MU_TYPE_HEADER;

  size_t argc;

  mu_type_member_t argv[/* argc */];
} mu_record_type_t;

const mu_record_type_t *mu_record_type(
    mu_engine_t *engine, size_t argc, const mu_type_member_t argv[/* argc */])
  __attribute__((malloc, nonnull(1)));

void mu_record_type_debug(const mu_record_type_t *type)
  __attribute__((nonnull));

#endif /* MU_STATOR_RECORD_TYPE_H */
