#ifndef MU_INDUCTOR_INDUCE_I
#define MU_INDUCTOR_INDUCE_I

#include <muon/stator.h>

#include "detect.h"
#include "../status.h"

#include <assert.h>
#include <stddef.h>

typedef enum {
  MU_BOOLEAN_HEAD,
  MU_INTEGER_HEAD,
  MU_LAMBDA_HEAD,
  MU_VECTOR_HEAD,
} mu_head_kind_t;

typedef enum {
  MU_CONTRAVARIANCE,
  MU_COVARIANCE,
  MU_INVARIANCE,
} mu_variance_t;

typedef struct {
  mu_head_kind_t kind;
  size_t argc;
  mu_variance_t variance[/* argc */];
} mu_head_t;

typedef struct type_t type_t;

typedef struct {
  const mu_name_t *name;
  const type_t *type;
} type_member_t;

struct type_t {
  enum {
    SIMPLE_TYPE,
    RECORD_TYPE,
  } kind;

  union {
    struct {
      const mu_head_t *head;
      const type_t *argv[/* head->argc */];
    };

    struct {
      size_t argc;
      type_member_t schema[];
    };
  };
};

typedef struct {
  const mu_type_t *lower;
  const mu_type_t *upper;
} induce_sub_t;

typedef struct {
  mu_engine_t *engine;
  mu_status_t *status;

  const detect_result_t *detect;

  size_t node_length;
  const mu_type_t **node_to_type;  /* const mu_type_t *[node_length] */

  size_t sub_volume;
  size_t sub_length;
  induce_sub_t *sub_data;

  const mu_type_t *next_a;
  const mu_type_t *next_b;
} induce_t;

extern _Thread_local induce_t *debug_induce;

/// Initialize the @a inductor to handle nodes and types in the @a engine
induce_t *induce_initialize(
    induce_t *induce,
    mu_engine_t *engine,
    mu_status_t *status,
    const detect_t *detect)
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

#endif /* MU_INDUCTOR_INDUCE_I */
