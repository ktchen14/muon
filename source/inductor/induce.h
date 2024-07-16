#ifndef MU_INDUCTOR_INDUCE_I
#define MU_INDUCTOR_INDUCE_I

#include <muon/stator.h>

#include "../status.h"

#include <assert.h>
#include <stddef.h>

typedef struct inductor_t inductor_t;
typedef struct inductor_t induce_t;

struct inductor_t {
  mu_engine_t *engine;
  mu_status_t *status;

  const mu_stmt_t *const *node_to_stmt;

  size_t node_number;
  size_t length;
  const mu_type_t **data; /* const mu_type_t *[length] */
};

/// Initialize the @a inductor to handle nodes and types in the @a engine
inductor_t *inductor_initialize(
    inductor_t *inductor,
    mu_engine_t *engine,
    const mu_stmt_t *const *node_to_stmt,
    mu_status_t *status)
  __attribute__((nonnull));

void inductor_raze(inductor_t *inductor) __attribute__((nonnull));

/// Return the type of the @a node in the @a induce context
__attribute__((nonnull, pure, returns_nonnull))
static inline const mu_type_t *induce_evince(
    const induce_t *induce, const mu_node_t *node) {
  assert(node->as_stator.id < induce->node_number);
  const mu_type_t *type = induce->data[node->as_stator.id];
  assert(type != NULL);
  return type;
}

/**
 * @brief Return the type of the @a node
 *
 * This will traverse each node reachable from the @a node and will add all
 * constraints to the @a inductor.
 */
const mu_type_t *induce_node(inductor_t *inductor, const mu_node_t *node)
  __attribute__((nonnull));

#endif /* MU_INDUCTOR_INDUCE_I */
