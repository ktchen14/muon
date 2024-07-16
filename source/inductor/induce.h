#ifndef MU_INDUCTOR_INDUCE_I
#define MU_INDUCTOR_INDUCE_I

#include <muon/stator.h>

#include "../status.h"

#include <assert.h>
#include <stddef.h>

typedef struct {
  mu_engine_t *engine;
  mu_status_t *status;

  const mu_stmt_t *const *node_to_stmt;

  size_t node_number;
  size_t length;
  const mu_type_t **data; /* const mu_type_t *[length] */
} induce_t;

/// Initialize the @a inductor to handle nodes and types in the @a engine
induce_t *induce_initialize(
    induce_t *induce,
    mu_engine_t *engine,
    const mu_stmt_t *const *node_to_stmt,
    mu_status_t *status)
  __attribute__((nonnull));

/**
 * @brief Return the type of the @a node within the @a induce context
 *
 * The behavior is undefined if:
 *
 * - @a induce or @a node is @c NULL
 * - @a node isn't in the same engine as the @a induce context is initialized to
 *   operate on
 * - @a node was assigned to the engine after the @a induce context was
 *   initialized
 */
__attribute__((nonnull, pure, returns_nonnull))
static inline const mu_type_t *induce_evince(
    const induce_t *induce, const mu_node_t *node) {
  assert(node->as_stator.id < induce->node_number);

  const mu_type_t *type = induce->data[node->as_stator.id];
  assert(type != NULL);
  return type;
}

/// Get the next type equivalent to @a type in the @a induce context
const mu_type_t *induce_get(const induce_t *induce, const mu_type_t *type)
  __attribute__((nonnull, pure));

/**
 * @brief Return the type of the @a node
 *
 * This will traverse each node reachable from the @a node and will add all
 * constraints to the @a inductor.
 */
const mu_type_t *induce_node(induce_t *inductor, const mu_node_t *node)
  __attribute__((nonnull));

#endif /* MU_INDUCTOR_INDUCE_I */
