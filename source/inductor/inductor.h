#ifndef MU_INDUCTOR_INDUCTOR_I
#define MU_INDUCTOR_INDUCTOR_I

#include <muon/stator.h>

#include "../status.h"

#include <assert.h>
#include <stddef.h>

typedef struct inductor_t inductor_t;

struct inductor_t {
  mu_engine_t *engine;

  size_t node_number;
  size_t length;
  const mu_type_t **induce; /* const mu_type_t *[length] */
  const mu_type_t **reduce;

  const mu_stmt_t *const *node_to_stmt;

  mu_status_t *status;
};

/// Initialize the @a inductor to handle nodes and types in the @a engine
inductor_t *inductor_initialize(
    inductor_t *inductor,
    mu_engine_t *engine,
    const mu_stmt_t *const *node_to_stmt,
    mu_status_t *status)
  __attribute__((nonnull));

void inductor_raze(inductor_t *inductor) __attribute__((nonnull));

/**
 * @brief Return the type of the @a node
 *
 * This will traverse each node reachable from the @a node and will add all
 * constraints to the @a inductor.
 */
const mu_type_t *induce_node(inductor_t *inductor, const mu_node_t *node)
  __attribute__((nonnull));

const mu_type_t *reduce_type(inductor_t *inductor, const mu_type_t *type)
  __attribute__((nonnull));

const mu_type_t *reduce_node(inductor_t *inductor, const mu_node_t *node)
  __attribute__((nonnull));

const mu_type_t *reduce_type_result(inductor_t *inductor, const mu_type_t *type);

#endif /* MU_INDUCTOR_I */
