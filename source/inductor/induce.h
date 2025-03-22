#ifndef MU_INDUCTOR_INDUCE_I
#define MU_INDUCTOR_INDUCE_I

#include <muon/stator.h>

#include "coercion.h"
#include "core.h"
#include "type.h"
#include "universe.h"

#include "detect.h"
#include "../status.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

typedef struct induce_t induce_t;
struct induce_t {
  mu_engine_t *engine;
  mu_status_t *status;

  const detect_result_t *detect;

  size_t type_number;

  size_t node_length;

  const mu_type_t **node_to_type; /* const type_t *[node_length] */

  universe_t universe;

  const mu_core_t *boolean_core;
  const mu_core_t *integer_core;
  const mu_core_t *lambda_core;
  const mu_core_t *vector_core;

  const mu_core_t *datatype_core;
  const mu_core_t *core[200];
  size_t core_length;

  const mu_core_t *record_core[200];
  size_t record_core_length;

  const record_instance_t *record_instance[200];
  size_t record_instance_length;

  const mu_id_coercion_t *id_coercion;

  const mu_type_t *aux[2000];
  const mu_coercion_t *coercion[2000];
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
  assert(node->id < induce->node_length);
  const mu_type_t *type = induce->node_to_type[node->id];
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

const mu_type_t *reduce_node(induce_t *induce, const mu_node_t *root);

/**
 * @brief Restrict type @a a to be a subtype of @a b in the @a induce engine
 *
 * On allocation failure, @c errno is set by the allocator. This function can't
 * fail otherwise. The behavior is undefined if:
 *
 * - @a induce, @a a, or @a b is @c NULL
 *
 * This returns &SELF if @a a and @a b are identical.
 *
 * @param induce the induce engine to restrict @a a and @a b within
 * @param a the type to restrict to a subtype of @a b
 * @param b the type to restrict to a supertype of @a a
 */

const induce_edge_t *restrict_type(
    induce_t *induce, const mu_type_t *a, const mu_type_t *b)
  __attribute__((nonnull));

extern const mu_name_t *vector_access;
extern const mu_name_t *vector_join;

#endif /* MU_INDUCTOR_INDUCE_I */
