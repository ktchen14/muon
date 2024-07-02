#ifndef MU_NODE_I
#define MU_NODE_I

#include "node/common.h"

#include "expr.h"  // IWYU pragma: export
#include "sign.h"  // IWYU pragma: export
#include "stmt.h"  // IWYU pragma: export

#include "inductor.h"

#include <stddef.h>
#include <stdlib.h>

static inline const mu_node_t *node_at(const mu_node_t *node, size_t i) {
  switch (node->kind) {
    case MU_ACCESS_EXPR_NODE: 
      return access_expr_at((const mu_access_expr_t *) node, i);

    case MU_INTEGER_EXPR_NODE:
      return integer_expr_at((const mu_integer_expr_t *) node, i);

    case MU_MEMBER_EXPR_NODE:
      return member_expr_at((const mu_member_expr_t *) node, i);

    case MU_NAME_EXPR_NODE:
      return name_expr_at((const mu_name_expr_t *) node, i);

    case MU_RECORD_EXPR_NODE:
      return record_expr_at((const mu_record_expr_t *) node, i);

    case MU_VECTOR_EXPR_NODE:
      return vector_expr_at((const mu_vector_expr_t *) node, i);

    case MU_ZERO_EXPR_NODE:
      return zero_expr_at((const mu_zero_expr_t *) node, i);

    case MU_INTEGER_SIGN_NODE:
    case MU_MEMBER_SIGN_NODE:
    case MU_NAME_SIGN_NODE:
    case MU_RECORD_SIGN_NODE:
    case MU_VARIABLE_SIGN_NODE:
    case MU_VECTOR_SIGN_NODE:
      abort();

    case MU_CONSTANT_STMT_NODE:
      return constant_stmt_at((const mu_constant_stmt_t *) node, i);

    case MU_TYPE_STMT_NODE:
      return type_stmt_at((const mu_type_stmt_t *) node, i);
  }

  return NULL;  // TODO: unreachable
}

static inline inductor_t *node_induce(
    const mu_node_t *node, inductor_t *inductor) {
  switch (node->kind) {
#define MU_EMIT(lower, upper, _) \
    case MU_##upper##_EXPR: \
      return lower##_expr_induce((const mu_##lower##_expr_t *) node, inductor);
    MU_EACH_EXPR_KIND(MU_EMIT)
#undef MU_EMIT

#define MU_EMIT(lower, upper, _) case MU_##upper##_SIGN: return inductor;
    MU_EACH_SIGN_KIND(MU_EMIT)
#undef MU_EMIT

#define MU_EMIT(lower, upper, _) case MU_##upper##_STMT: return inductor;
    MU_EACH_STMT_KIND(MU_EMIT)
#undef MU_EMIT
  }

  __builtin_unreachable();
}

/// Return the cursor in the @a node
__attribute__((const, nonnull, returns_nonnull))
static inline node_cursor_t *node_cursor(const mu_node_t *node) {
  node_header_t *header = (node_header_t *) (
      (char *) node - offsetof(node_header_t, data));
  return &header->cursor;
}

#include <assert.h>

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

#endif /* MU_NODE_I */
