#ifndef MU_INDUCTOR_INDUCE_I
#define MU_INDUCTOR_INDUCE_I

#include <muon/engine.h>
#include <muon/inductor/type.h>

#include "coercion.h"
#include "core.h"
#include "universe.h"

#include "detect.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

typedef struct {
  MuonCoercion *coercion;
  MuonType *source_type;
} induce_node_t;

typedef struct mu_scheme_t mu_scheme_t;
struct mu_scheme_t {
  const induce_t *induce;
  mu_scheme_t *parent;

  // The lowest id that a type that's a part of this scheme will have
  size_t id;
};

typedef struct induce_t mu_inductor_t;
typedef struct induce_t induce_t;
struct induce_t {
  MuonEngine *engine;
  size_t node_length;
  size_t type_number;
  size_t coercion_number;

  const detect_result_t *detect;

  induce_node_t *result;

  universe_t universe;

  MuonCore *boolean_core;
  MuonCore *integer_core;
  MuonCore *lambda_core;
  MuonCore *vector_core;

  MuonCore *datatype_core;
  MuonCore *core[200];
  size_t core_length;

  const record_instance_t *record_instance[200];
  size_t record_instance_length;

  const mu_instance_t *instance[200];
  size_t instance_length;

  MuonCoercion *id_coercion;
  MuonCoercion *slot_coercion;

  mu_scheme_t *scheme;
};

extern _Thread_local induce_t *debug_induce;

/// Initialize the @a inductor to handle nodes and types in the @a engine
induce_t *induce_initialize(
    induce_t *induce, MuonEngine *engine, const detect_t *detect)
  MUON_HINT_SUFFIX(nonnull);

MUON_HINT(nonnull, pure, returns_nonnull)
static inline MuonType *node_type(const induce_t *induce, MuonNode *node) {
  assert(node->engine == induce->engine);
  assert(node->id < induce->node_length);

  induce_node_t result = induce->result[node->id];
  assert(result.source_type != NULL);
  return result.source_type;
}

MUON_HINT(nonnull, pure)
static inline MuonCoercion *node_coercion(
    const induce_t *induce, MuonNode *node) {
  assert(node->engine == induce->engine);
  assert(node->id < induce->node_length);

  induce_node_t result = induce->result[node->id];
  return result.coercion;
}

MUON_HINT(nonnull)
static inline void override_coercion(
    induce_t *induce, MuonNode *node, MuonCoercion *coercion) {
  assert(node->engine == induce->engine);
  assert(node->id < induce->node_length);
  induce_node_t *result = &induce->result[node->id];
  result->coercion = coercion;
}

/**
 * @brief Return the type of the @a node
 *
 * This will traverse each node reachable from the @a node and will add all
 * constraints to the @a inductor.
 */
MuonType *induce_node(induce_t *inductor, MuonNode *node)
  MUON_HINT_SUFFIX(nonnull);

MuonType *reduce_node(induce_t *induce, MuonNode *root);

/**
 * @brief If the coercion source => target is ensured, then return it
 */
MuonCoercion *retrieve_coercion(
    induce_t *induce, MuonType *source, MuonType *target)
  MUON_HINT_SUFFIX(nonnull);

MuonCoercion *coerce_node(induce_t *induce, MuonNode *node, MuonType *target)
  MUON_HINT_SUFFIX(nonnull);

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
MuonCoercion *ensure_coercion(
    induce_t *induce, MuonType *source, MuonType *target)
  MUON_HINT_SUFFIX(nonnull);

MuonType *generalize_type(induce_t *induce, MuonType *root)
  MUON_HINT_SUFFIX(nonnull);

mu_scheme_t *mu_scheme(mu_scheme_t *parent)
  MUON_HINT_SUFFIX(malloc);

extern MuonName *vector_access;
extern MuonName *vector_join;

#endif /* MU_INDUCTOR_INDUCE_I */
