#ifndef MU_STATOR_VARIABLE_VIEW_I
#define MU_STATOR_VARIABLE_VIEW_I

#include <muon/stator/variable_view.h>  // IWYU pragma: export

#include "abstract_node.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline const mu_node_t *variable_view_at(
    const mu_variable_view_t *view, size_t i) {
  return NULL;
}

#endif /* MU_STATOR_VARIABLE_VIEW_I */
