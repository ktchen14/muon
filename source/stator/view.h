#ifndef MU_STATOR_VIEW_I
#define MU_STATOR_VIEW_I

#include <muon/stator/view.h>  // IWYU pragma: export

#include <stddef.h>

__attribute__((const, nonnull))
static inline const mu_node_t *variable_view_at(
    const mu_variable_view_t *view, size_t i) {
  return NULL;
}

#endif /* MU_STATOR_VIEW_I */
