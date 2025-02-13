#ifndef MU_INDUCTOR_INDUCE_I
#define MU_INDUCTOR_INDUCE_I

#include <muon/stator.h>

#include "detect.h"
#include "../status.h"

#include <assert.h>
#include <stddef.h>

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

struct type_t {
  enum {
    SIMPLE_TYPE,
    RECORD_TYPE,
    VARIABLE_TYPE,
    SCHEME_TYPE,
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
      size_t number;

      // In let polymorphism, the right hand side of each let declaration is
      // evaluated within a separate type environment. The type environment consists
      // of all bindings that are known from the scope outside of the let
      // declaration.
      //
      // Since these type environments are nested, we can track the depth of them
      // using this level.
      //
      // When we exit the right hand side of a let declaration, we "seal" the type
      // of the rhs with the level of the type environment used at the time that it
      // was type checked. This means that every type variable with a higher level,
      // reachable from the output type, should be generalized.
      //
      // Essentially, whenever we see a type variable with a higher level than the
      // current level, that type variable is "sealed". This means that the upper
      // and lower bounds of that type variable will never be modified again.

      // This is the node that "owns" this variable
      const mu_node_t *scope;
    };

    // SCHEME_TYPE
    struct {
      const type_t *type;
      const mu_node_t *highest_scope;
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
