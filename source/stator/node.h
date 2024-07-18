#ifndef MU_STATOR_NODE_I
#define MU_STATOR_NODE_I

#include <muon/stator/node.h>  // IWYU pragma: export

#include "abstract_node.h"     // IWYU pragma: export

#include "access_expr.h"       // IWYU pragma: export
#include "boolean_expr.h"      // IWYU pragma: export
#include "integer_expr.h"      // IWYU pragma: export
#include "invoke_expr.h"       // IWYU pragma: export
#include "lambda_expr.h"       // IWYU pragma: export
#include "name_expr.h"         // IWYU pragma: export
#include "record_expr.h"       // IWYU pragma: export
#include "vector_expr.h"       // IWYU pragma: export
#include "zero_expr.h"         // IWYU pragma: export

#include "boolean_sign.h"      // IWYU pragma: export
#include "integer_sign.h"      // IWYU pragma: export
#include "member_sign.h"       // IWYU pragma: export
#include "name_sign.h"         // IWYU pragma: export
#include "record_sign.h"       // IWYU pragma: export
#include "variable_sign.h"     // IWYU pragma: export
#include "vector_sign.h"       // IWYU pragma: export

#include "define_stmt.h"       // IWYU pragma: export
#include "type_stmt.h"         // IWYU pragma: export

#include "variable_view.h"     // IWYU pragma: export

#include "engine.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>

typedef struct {
  const mu_node_t *anterior;
  size_t i;
} node_cursor_t;

typedef struct {
  node_cursor_t cursor;
  _Alignas(union {
#define MU_EMIT(lower, u, t) mu_##lower##_t lower;
    MU_EACH_NODE_KIND(MU_EMIT)
#undef MU_EMIT
  }) char data[];
} node_header_t;

/// @internal Allocate a node of size @a size in the @a engine
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

/// @internal Assign the abstract @a node to the @a engine
__attribute__((nonnull, returns_nonnull))
static inline mu_node_t *assign_node(mu_engine_t *engine, mu_node_t *node) {
  node->as_stator.engine = engine;
  node->as_stator.id = engine->node_number++;
  return node;
}

/// Assign the concrete @a node to the @a engine
#define assign_node(engine, node) \
  ((typeof((node))) (assign_node)((engine), &(node)->as_node))

/// Return the cursor attached to the @a node
__attribute__((const, nonnull, returns_nonnull))
static inline node_cursor_t *node_cursor(const mu_node_t *node) {
  node_header_t *header = (node_header_t *) (
      (char *) node - offsetof(node_header_t, data));
  return &header->cursor;
}

/// Continue into the node
static inline const mu_node_t *node_continue(
    const mu_node_t *node, const mu_node_t *next) {
  node_cursor_t *cursor = node_cursor(next);
  assert(cursor->anterior == NULL && cursor->i == 0);
  cursor->anterior = node;
  return next;
}

/// Return from the node
__attribute__((nonnull))
static inline const mu_node_t *node_return(const mu_node_t *node) {
  node_cursor_t *cursor = node_cursor(node);
  const mu_node_t *anterior = cursor->anterior;
  *cursor = (node_cursor_t) {0};
  return anterior;
}

/// Return the <em>i</em>th node in the abstract @a node
static inline const mu_node_t *node_at(const mu_node_t *node, size_t i) {
  switch (node->kind) {
#define MU_EMIT(lower, upper, t) \
    case MU_##upper##_NODE: \
      return lower##_at((const mu_##lower##_t *) node, i);
    MU_EACH_NODE_KIND(MU_EMIT)
#undef MU_EMIT
  }
  __builtin_unreachable();
}

#endif /* MU_STATOR_NODE_I */
