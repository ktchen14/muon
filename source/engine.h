#ifndef MU_ENGINE_I
#define MU_ENGINE_I

#include <muon/engine.h>  // IWYU pragma: export

#include "common.h"
#include "node/common.h"
#include "type.h"

/// Assign the abstract @a node to the @a engine
__attribute__((nonnull, returns_nonnull))
static inline mu_node_t *assign_node(mu_engine_t *engine, mu_node_t *node) {
  node->as_stator.engine = engine;
  node->as_stator.id = engine->node_number++;
  return node;
}

/// Assign the abstract @a type to the @a engine
__attribute__((nonnull, returns_nonnull))
static inline mu_type_t *assign_type(mu_engine_t *engine, mu_type_t *type) {
  type->as_stator.engine = engine;
  type->as_stator.id = engine->type_number++;
  return type;
}

/// Assign the concrete @a node to the @a engine
#define assign_node(engine, node) \
  ((typeof((node))) (assign_node)((engine), &(node)->as_node))

/// Assign the concrete @a type to the @a engine
#define assign_type(engine, type) \
  ((typeof((type))) (assign_type)((engine), &(type)->as_type))

#endif /* MU_ENGINE_I */
