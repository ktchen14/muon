#ifndef MU_STATOR_VARIABLE_VIEW_H
#define MU_STATOR_VARIABLE_VIEW_H

#include "abstract_node.h"
#include "name.h"

typedef struct {
  MU_VIEW_HEADER;

  const mu_name_t *name;
} mu_variable_view_t;

const mu_variable_view_t *mu_variable_view(
    mu_engine_t *engine, const mu_name_t *name)
  __attribute__((malloc, nonnull));

void mu_variable_view_debug(const mu_variable_view_t *view)
  __attribute__((nonnull));

#endif /* MU_STATOR_VARIABLE_VIEW_H */
