#ifndef MU_ENGINE_I
#define MU_ENGINE_I

#include <muon/engine.h>  // IWYU pragma: export

#include "common.h"
#include "stator.h"

#include <stddef.h>
#include <errno.h>
#include <stdlib.h>

/// Allocate a stator of the @a size in the @a engine
__attribute__((malloc, nonnull))
static inline void *engine_allocate(mu_engine_t *engine, size_t size) {
  return malloc(size);
}

__attribute__((malloc, nonnull))
static inline void *node_allocate(mu_engine_t *engine, size_t size) {
  if (rare((size = struct_size(node_header_t, data, size)) == 0))
    return errno = ENOMEM, NULL;

  node_header_t *header;
  if ((header = engine_allocate(engine, size)) == NULL)
    return NULL;
  *header = (node_header_t) {0};

  return header->data;
}

__attribute__((malloc, nonnull))
static inline void *type_allocate(mu_engine_t *engine, size_t size) {
  if (rare((size = struct_size(type_header_t, data, size)) == 0))
    return errno = ENOMEM, NULL;

  type_header_t *header;
  if ((header = engine_allocate(engine, size)) == NULL)
    return NULL;
  *header = (type_header_t) {0};

  return header->data;
}

/// Assign the @a name to the @a engine
__attribute__((nonnull))
static inline mu_name_t *assign_name(mu_engine_t *engine, mu_name_t *name) {
  name->as_stator.engine = engine;
  name->as_stator.id = engine->name_number++;
  engine->name[name->as_stator.id] = name;
  return name;
}

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
