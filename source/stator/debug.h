#ifndef MU_STATOR_DEBUG_I
#define MU_STATOR_DEBUG_I

#include "node.h"
#include "type.h"

#include "../inductor/induce.h"

#include <stdio.h>

/// The amount of indentation to insert before each line of debug output
extern _Thread_local int debug_indent;

/// Whether to debug a type as a negative or positive type
extern _Thread_local _Bool debug_negate;

#define WITH_DEBUG_INDENT() \
  for (int _i = (debug_indent += 2); debug_indent == _i; debug_indent -= 2)

#define WITH_DEBUG_NEGATE() \
  for (_Bool _n = (debug_negate = !debug_negate); debug_negate == _n; debug_negate = !debug_negate)

__attribute__((nonnull))
static inline void debug_ambinode_type(const mu_node_t *node) {
  if (debug_induce == NULL)
    return;

  const mu_type_t *type;
  if ((type = induce_evince(debug_induce, node)) == NULL)
    return;

  _Bool negate = debug_negate;

  debug_negate = 0;
  fputs(" ∷ either (", stderr);
  mu_type_debug(type);
  fputs(")⁺", stderr);

  debug_negate = 1;
  fputs(" or (", stderr);
  mu_type_debug(type);
  fputs(")⁻", stderr);

  debug_negate = negate;
}

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
