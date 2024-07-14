#ifndef MU_STATOR_MEMBER_TYPE_H
#define MU_STATOR_MEMBER_TYPE_H

#include "abstract_type.h"

#include "name.h"

typedef struct {
  MU_TYPE_HEADER;

  const mu_name_t *name;
  const mu_type_t *matter;
} mu_member_type_t;

const mu_member_type_t *mu_member_type(
    mu_engine_t *engine, const mu_name_t *name, const mu_type_t *matter)
  __attribute__((malloc, nonnull));

void mu_member_type_debug(const mu_member_type_t *type)
  __attribute__((nonnull));

#endif /* MU_STATOR_MEMBER_TYPE_H */
