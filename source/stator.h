#ifndef MU_STATOR_I
#define MU_STATOR_I

#include <muon/stator.h>           // IWYU pragma: export

#include "stator/common.h"         // IWYU pragma: export
#include "stator/name.h"           // IWYU pragma: export
#include "stator/node.h"           // IWYU pragma: export
#include "stator/type.h"           // IWYU pragma: export

#include "stator/access_expr.h"    // IWYU pragma: export
#include "stator/integer_expr.h"   // IWYU pragma: export
#include "stator/member_expr.h"    // IWYU pragma: export
#include "stator/name_expr.h"      // IWYU pragma: export
#include "stator/record_expr.h"    // IWYU pragma: export
#include "stator/vector_expr.h"    // IWYU pragma: export
#include "stator/zero_expr.h"      // IWYU pragma: export

#include "stator/integer_sign.h"   // IWYU pragma: export
#include "stator/member_sign.h"    // IWYU pragma: export
#include "stator/name_sign.h"      // IWYU pragma: export
#include "stator/record_sign.h"    // IWYU pragma: export
#include "stator/variable_sign.h"  // IWYU pragma: export
#include "stator/vector_sign.h"    // IWYU pragma: export

#include "stator/constant_stmt.h"  // IWYU pragma: export
#include "stator/type_stmt.h"      // IWYU pragma: export

#include "stator/integer_type.h"   // IWYU pragma: export
#include "stator/variable_type.h"  // IWYU pragma: export
#include "stator/vector_type.h"    // IWYU pragma: export

#include <assert.h>
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

typedef struct {
  const mu_type_t *anterior;
  size_t i;
} type_cursor_t;

typedef struct {
  type_cursor_t cursor;
  _Alignas(union {
#define MU_EMIT(lower, u, t) mu_##lower##_type_t lower;
    MU_EACH_TYPE_KIND(MU_EMIT)
#undef MU_EMIT
  }) char data[];
} type_header_t;

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

/// Return the cursor attached to the @a type
__attribute__((const, nonnull, returns_nonnull))
static inline type_cursor_t *type_cursor(const mu_type_t *type) {
  type_header_t *header = (type_header_t *) (
      (char *) type - offsetof(type_header_t, data));
  return &header->cursor;
}

/// Continue into the type
static inline const mu_type_t *type_continue(
    const mu_type_t *type, const mu_type_t *next) {
  type_cursor_t *cursor = type_cursor(next);
  assert(cursor->anterior == NULL && cursor->i == 0);
  cursor->anterior = type;
  return next;
}

/// Return from the type
__attribute__((nonnull))
static inline const mu_type_t *type_return(const mu_type_t *type) {
  type_cursor_t *cursor = type_cursor(type);
  const mu_type_t *anterior = cursor->anterior;
  *cursor = (type_cursor_t) {0};
  return anterior;
}

__attribute__((nonnull, pure))
static inline const mu_type_t *type_at(const mu_type_t *type, size_t i) {
#define MU_EMIT(lower, upper, _) \
    case MU_##upper##_TYPE: \
      return lower##_type_at((const mu_##lower##_type_t *) type, i);
  switch (type->kind) { MU_EACH_TYPE_KIND(MU_EMIT) }
#undef MU_EMIT

  __builtin_unreachable();
}

__attribute__((nonnull))
static inline const mu_type_t *type_reduce(
    const mu_type_t *type, inductor_t *inductor) {
#define MU_EMIT(lower, upper, _) \
    case MU_##upper##_TYPE: \
      return &lower##_type_reduce((const mu_##lower##_type_t *) type, inductor)->as_type;
  switch (type->kind) { MU_EACH_TYPE_KIND(MU_EMIT) }
#undef MU_EMIT

  __builtin_unreachable();
}

#endif /* MU_STATOR_I */
