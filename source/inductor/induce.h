#ifndef MU_INDUCTOR_INDUCE_I
#define MU_INDUCTOR_INDUCE_I

#include <muon/stator.h>

#include "core.h"
#include "type.h"

#include "detect.h"
#include "../status.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

typedef const char *coercion_t;

typedef struct {
  const mu_type_t *lower;
  const mu_type_t *upper;
  coercion_t coercion;
} induce_edge_t;

typedef struct induce_t induce_t;
struct induce_t {
  mu_engine_t *engine;
  mu_status_t *status;

  const detect_result_t *detect;

  size_t node_length;

  const mu_type_t **node_to_type; /* const type_t *[node_length] */

  size_t edge_volume;
  size_t edge_length;
  induce_edge_t *edge;

  const mu_core_t *boolean_core;
  const mu_core_t *integer_core;
  const mu_core_t *lambda_core;
  const mu_core_t *vector_core;
};

typedef struct open_scheme_t open_scheme_t;
struct open_scheme_t {
  induce_t *induce;
  const mu_node_t *node;
  open_scheme_t *parent;
  size_t rank;
  mu_variable_type_t *link;
};

extern _Thread_local induce_t *debug_induce;

/// Initialize the @a inductor to handle nodes and types in the @a engine
induce_t *induce_initialize(
    induce_t *induce,
    mu_engine_t *engine,
    mu_status_t *status,
    const detect_t *detect)
  __attribute__((nonnull));

__attribute__((nonnull, pure, returns_nonnull))
static inline const mu_type_t *induce_reveal(
    const induce_t *induce, const mu_node_t *node) {
  assert(node->as_stator.id < induce->node_length);
  const mu_type_t *type = induce->node_to_type[node->as_stator.id];
  assert(type != NULL);
  return type;
}

/**
 * @brief Return the type of the @a node
 *
 * This will traverse each node reachable from the @a node and will add all
 * constraints to the @a inductor.
 */
const mu_type_t *induce_node(induce_t *inductor, const mu_node_t *node)
  __attribute__((nonnull));

void mark_type_from_anywhere_first(induce_t *induce, const mu_type_t *root, type_link_t *link);

#endif /* MU_INDUCTOR_INDUCE_I */
