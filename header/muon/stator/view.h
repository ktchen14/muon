#ifndef MU_STATOR_VIEW_H
#define MU_STATOR_VIEW_H

#include "abstract_node.h"

#include "name.h"

/// The header that each concrete view must have
#define MU_VIEW_HEADER union { \
  mu_view_t as_view; \
  mu_node_t as_node; \
  mu_stator_t as_stator; \
}

typedef struct {
  MU_VIEW_HEADER;
  const mu_name_t *name;
} mu_variable_view_t;

const mu_variable_view_t *mu_variable_view(
    mu_engine_t *engine, const mu_name_t *name)
  __attribute__((malloc, nonnull));

void mu_variable_view_debug(const mu_variable_view_t *view)
  __attribute__((nonnull));

#endif /* MU_STATOR_VIEW_H */
