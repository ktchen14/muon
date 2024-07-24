#ifndef MU_STATOR_DEBUG_I
#define MU_STATOR_DEBUG_I

#include "node.h"
#include "type.h"

#include "../inductor/induce.h"

#include <stdio.h>

__attribute__((nonnull))
static inline void debug_node_type(const mu_node_t *node) {
  if (debug_induce == NULL)
    return;

  const mu_type_t *type;
  if ((type = induce_evince(debug_induce, node)) == NULL)
    return;

  fprintf(stderr, " ∷ ");
  mu_type_debug(type);
}

#endif /* MU_STATOR_DEBUG_I */
