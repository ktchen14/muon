#ifndef MU_INDUCTOR_I
#define MU_INDUCTOR_I

#include <muon/stator.h>

#include "status.h"

#include <assert.h>
#include <stddef.h>

typedef struct inductor_t inductor_t;

struct inductor_t {
  mu_engine_t *engine;

  size_t node_number;
  size_t length;
  const mu_type_t **data; /* const mu_type_t *[length] */

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
 * @brief Return the type of the @a node in the @a inductor
 *
 * The result is an rvalue.
 */
#define inductor_node(inductor, node) (*({ \
    inductor_t *_inductor = (inductor); \
    const mu_node_t *_node = (node); \
    assert(_node->as_stator.id < _inductor->node_number); \
    &_inductor->data[_node->as_stator.id]; \
  }))

const mu_type_t *inductor_root(inductor_t *inductor, const mu_type_t *type);

#define inductor_type_root inductor_root

/// Return the archtype of the @a node in the @a inductor
const mu_type_t *inductor_type_of_node(
    inductor_t *inductor, const mu_node_t *node)
  __attribute__((nonnull));

/// Equate type @a a to type @a b in the @a inductor
inductor_t *inductor_equate(
    inductor_t *inductor, const mu_type_t *a, const mu_type_t *b)
  __attribute__((nonnull));

#endif /* MU_INDUCTOR_I */
