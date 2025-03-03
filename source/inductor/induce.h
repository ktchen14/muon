#ifndef MU_INDUCTOR_INDUCE_I
#define MU_INDUCTOR_INDUCE_I

#include <muon/stator.h>

#include "detect.h"
#include "../status.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

typedef enum {
  MU_BOOLEAN_CORE,
  MU_INTEGER_CORE,
  MU_LAMBDA_CORE,
  MU_VECTOR_CORE,
} mu_core_kind_t;

typedef enum {
  MU_COVARIANCE,
  MU_CONTRAVARIANCE,
  MU_INVARIANCE,
} mu_variance_t;

typedef struct {
  mu_core_kind_t kind;
  size_t argc;
  mu_variance_t variance[/* argc */];
} mu_core_t;

typedef struct type_t type_t;

typedef struct {
  const mu_name_t *name;
  const type_t *type;
} type_member_t;

/// Compare the type member @a a to the type member @a b
__attribute__((nonnull, pure))
static inline int type_member_cmp(const void *a, const void *b) {
  const type_member_t *ra = a, *rb = b;
  assert(ra->name != NULL && rb->name != NULL);
  return name_cmp(ra->name, rb->name);
}

struct type_t {
  enum {
    SIMPLE_TYPE,
    RECORD_TYPE,
    VARIABLE_TYPE,
    SCHEME_TYPE,
    JOIN_TYPE,
  } kind;

  union {
    // SIMPLE_TYPE
    struct {
      const mu_core_t *core;
      const type_t *argv[/* core->argc */];
    };

    // RECORD_TYPE
    struct {
      size_t argc;
      type_member_t schema[];
    };

    // VARIABLE_TYPE
    struct {
      const type_t *next;

      // Used to generate a name
      size_t number;

      size_t rank;
      const type_t *polymorphic_to;

      _Bool positively_reachable;
      _Bool negatively_reachable;
    };

    // SCHEME_TYPE
    struct {
      const type_t *matter;
      size_t polymorphic_length;
      const type_t *polymorphic[];
    };

    // JOIN_TYPE
    struct {
      size_t join_argc;
      const type_t *join_argv[];
    };
  };
};

typedef struct {
  const type_t *lower;
  const type_t *upper;
} induce_sub_t;

typedef struct {
  mu_engine_t *engine;
  mu_status_t *status;

  const detect_result_t *detect;

  size_t node_length;
  const mu_type_t **node_to_type;  /* const mu_type_t *[node_length] */

  const type_t **node_to_type_actual; /* const type_t *[node_length] */

  size_t sub_volume;
  size_t sub_length;
  induce_sub_t *sub_data;

  const mu_core_t *boolean_core;
  const mu_core_t *integer_core;
  const mu_core_t *lambda_core;
  const mu_core_t *vector_core;

  /* Map of each define stmt to its parent define stmt */
  const mu_node_t **define_stmt_map;
} induce_t;

typedef struct open_scheme_t open_scheme_t;
struct open_scheme_t {
  induce_t *induce;
  const mu_node_t *node;
  open_scheme_t *parent;
  size_t rank;
  const type_t *link;
};

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

__attribute__((nonnull, pure, returns_nonnull))
static inline const type_t *induce_reveal(
    const induce_t *induce, const mu_node_t *node) {
  assert(node->as_stator.id < induce->node_length);
  const type_t *type = induce->node_to_type_actual[node->as_stator.id];
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

void debug_type(const type_t *type);

#endif /* MU_INDUCTOR_INDUCE_I */
