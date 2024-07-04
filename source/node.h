#ifndef MU_NODE_I
#define MU_NODE_I

#include <muon/node.h>           // IWYU pragma: export

#include "node/access_expr.h"    // IWYU pragma: export
#include "node/integer_expr.h"   // IWYU pragma: export
#include "node/member_expr.h"    // IWYU pragma: export
#include "node/name_expr.h"      // IWYU pragma: export
#include "node/record_expr.h"    // IWYU pragma: export
#include "node/vector_expr.h"    // IWYU pragma: export
#include "node/zero_expr.h"      // IWYU pragma: export

#include "node/integer_sign.h"   // IWYU pragma: export
#include "node/member_sign.h"    // IWYU pragma: export
#include "node/name_sign.h"      // IWYU pragma: export
#include "node/record_sign.h"    // IWYU pragma: export
#include "node/variable_sign.h"  // IWYU pragma: export
#include "node/vector_sign.h"    // IWYU pragma: export

#include "node/constant_stmt.h"  // IWYU pragma: export
#include "node/type_stmt.h"      // IWYU pragma: export

#include "inductor.h"
#include "stator.h"

#include <assert.h>
#include <stddef.h>

typedef struct {
  const mu_node_t *anterior;
  size_t i;
} node_cursor_t;

typedef struct {
  node_cursor_t cursor;
  _Alignas(max_align_t) char data[];
} node_header_t;

/// Return the cursor in the @a node
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


static inline const mu_node_t *node_at(const mu_node_t *node, size_t i) {
  switch (node->kind) {
#define MU_EMIT(lower, upper, t) \
    case MU_##upper##_NODE: \
      return lower##_at((const mu_##lower##_t *) node, i);
    MU_EACH_NODE_KIND(MU_EMIT)
#undef MU_EMIT
  }
  assert(0);
}

static inline inductor_t *node_induce(
    const mu_node_t *node, inductor_t *inductor) {
  switch (node->kind) {
#define MU_EMIT(lower, upper, t) \
    case MU_##upper##_EXPR_NODE: \
      return lower##_expr_induce((const mu_##lower##_expr_t *) node, inductor);
    MU_EACH_EXPR_KIND(MU_EMIT)
#undef MU_EMIT

#define MU_EMIT(lower, upper, t) case MU_##upper##_SIGN: return inductor;
    MU_EACH_SIGN_KIND(MU_EMIT)
#undef MU_EMIT

    case MU_CONSTANT_STMT_NODE:
      return constant_stmt_induce((const mu_constant_stmt_t *) node, inductor);

    case MU_TYPE_STMT_NODE:
      return inductor;
  }

  __builtin_unreachable();
}

#endif /* MU_NODE_I */
