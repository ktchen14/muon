#ifndef MU_INDUCTOR_INDUCE_I
#define MU_INDUCTOR_INDUCE_I

#include <muon/stator.h>

#include "coercion.h"
#include "core.h"
#include "type.h"
#include "universe.h"

#include "detect.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

typedef struct {
  const mu_coercion_t *coercion;
  const mu_type_t *target;
} node_coercion_t;

typedef struct induce_t induce_t;
struct induce_t {
  mu_engine_t *engine;
  size_t node_length;
  size_t type_number;

  const detect_result_t *detect;

  const mu_type_t **node_to_type; /* const type_t *[node_length] */
  node_coercion_t *node_coercion;

  universe_t universe;

  const mu_core_t *boolean_core;
  const mu_core_t *integer_core;
  const mu_core_t *lambda_core;
  const mu_core_t *vector_core;

  const mu_core_t *datatype_core;
  const mu_core_t *core[200];
  size_t core_length;

  const record_instance_t *record_instance[200];
  size_t record_instance_length;

  const mu_instance_t *instance[200];
  size_t instance_length;

  const mu_coercion_t *id_coercion;
  const mu_coercion_t *slot_coercion;

  mu_scheme_t *scheme;
};

extern _Thread_local induce_t *debug_induce;

/// Initialize the @a inductor to handle nodes and types in the @a engine
induce_t *induce_initialize(
    induce_t *induce, mu_engine_t *engine, const detect_t *detect)
  __attribute__((nonnull));

__attribute__((nonnull, pure, returns_nonnull))
static inline const mu_type_t *evince_type(
    const induce_t *induce, const mu_node_t *node) {
  assert(node->id < induce->node_length);
  const mu_type_t *type = induce->node_to_type[node->id];
  assert(type != NULL);
  return type;
}

__attribute__((nonnull, pure, returns_nonnull))
static inline const mu_coercion_t *evince_coercion(
    const induce_t *induce, const mu_node_t *node, const mu_type_t **target) {
  assert(node->id < induce->node_length);
  node_coercion_t node_coercion = induce->node_coercion[node->id];
  assert(node_coercion.coercion != NULL);
  assert(node_coercion.target != NULL);
  *target = node_coercion.target;
  return node_coercion.coercion;
}

/**
 * @brief Assign the @a coercion to the @a node
 *
 * Logically, the assigned @a coercion occurs to the value returned when the
 * node is evaluated.
 *
 * The behavior is undefined if:
 * - @a node and @a induce don't have the same @a engine
 * - @a node was created after @a induce
 * - a coercion has already been assigned to the @a node
 */
__attribute__((nonnull))
static inline void assign_coercion(
    induce_t *induce,
    const mu_node_t *node,
    const mu_coercion_t *coercion,
    const mu_type_t *target) {
  assert(node->engine == induce->engine);
  assert(node->id < induce->node_length);

  node_coercion_t *node_coercion = &induce->node_coercion[node->id];
  assert(node_coercion->coercion == NULL);
  assert(node_coercion->target == NULL);

  *node_coercion = (node_coercion_t) { .coercion = coercion, .target = target };
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
 * @brief If the coercion source => target is ensured, then return it
 */
const mu_coercion_t *retrieve_coercion(
    induce_t *induce, const mu_type_t *source, const mu_type_t *target)
  __attribute__((nonnull));

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
// Ensure that a coercion exists, and will always exist, from source => target.
// Return that coercion.
const mu_coercion_t *ensure_coercion(
    induce_t *induce, const mu_type_t *source, const mu_type_t *target)
  __attribute__((nonnull));

const mu_type_t *generalize_type(induce_t *induce, const mu_type_t *type)
  __attribute__((nonnull));

extern const mu_name_t *vector_access;
extern const mu_name_t *vector_join;

#endif /* MU_INDUCTOR_INDUCE_I */
