#ifndef MU_STATOR_NODE_I
#define MU_STATOR_NODE_I

#include <muon/stator/node.h>  // IWYU pragma: export

#include "../common.h"
#include "name.h"

#include <assert.h>
#include <stddef.h>

/// @internal An enumeration over each kind of node, e.g. @c _access_expr_kind
enum {
#define MU_EMIT(lower, u, t) _##lower##_kind,
  MU_EACH_NODE_KIND(MU_EMIT)
#undef MU_EMIT
};

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
  switch ON_ABSTRACT_OBJECT(node) {
    case MU_BOOLEAN_EXPR:
    case MU_INTEGER_EXPR:
    case MU_NAME_EXPR:
    case MU_NATIVE_EXPR:
    case MU_ZERO_EXPR:
    case MU_BOOLEAN_SIGN:
    case MU_INTEGER_SIGN:
    case MU_NAME_SIGN:
    case MU_DATATYPE_OPTION:
    case MU_VARIABLE_VIEW:
      return NULL;

    case IS_KIND_OF(access_expr):
      return (const mu_node_t *[]) { &access_expr->matter->as_node, NULL }[i];

    case IS_KIND_OF(invoke_expr):
      return (const mu_node_t *[]) {
        &invoke_expr->operator->as_node, &invoke_expr->argument->as_node, NULL,
      }[i];

    case IS_KIND_OF(lambda_expr):
      return (const mu_node_t *[]) {
        &lambda_expr->argument->as_node, &lambda_expr->matter->as_node, NULL,
      }[i];

    case IS_KIND_OF(expr_member):
      return (const mu_node_t *[]) { &expr_member->expr->as_node, NULL }[i];

    case IS_KIND_OF(record_expr):
      return i < record_expr->argc ? &record_expr->argv[i]->as_node : NULL;

    case IS_KIND_OF(sequence_expr):
      return i < sequence_expr->argc ? &sequence_expr->argv[i]->as_node : NULL;

    case IS_KIND_OF(switch_case):
      return (const mu_node_t *[]) { &switch_case->expr->as_node, NULL }[i];

    case IS_KIND_OF(switch_expr):
      return i < switch_expr->argc ? &switch_expr->argv[i]->as_node : NULL;

    case IS_KIND_OF(vector_expr):
      return i < vector_expr->argc ? &vector_expr->argv[i]->as_node : NULL;

    case IS_KIND_OF(record_sign):
      return i < record_sign->argc ? &record_sign->argv[i].sign->as_node : NULL;

    case IS_KIND_OF(vector_sign):
      return (const mu_node_t *[]) { &vector_sign->matter->as_node, NULL }[i];

    case IS_KIND_OF(define_stmt):
      return (const mu_node_t *[]) { &define_stmt->expr->as_node, NULL }[i];

    case IS_KIND_OF(datatype_stmt):
      return i < datatype_stmt->argc ? &datatype_stmt->argv[i]->as_node : NULL;
  }
  __builtin_unreachable();
}

mu_record_expr_t *record_expr_allocate(mu_engine_t *engine, size_t argc)
  __attribute__((malloc, nonnull));

const mu_record_expr_t *record_expr_activate(mu_record_expr_t *expr)
  __attribute__((nonnull));

mu_switch_expr_t *switch_expr_allocate(mu_engine_t *engine, size_t argc)
  __attribute__((malloc, nonnull));

const mu_switch_expr_t *switch_expr_activate(mu_switch_expr_t *expr)
  __attribute__((nonnull));

mu_sequence_expr_t *sequence_expr_allocate(mu_engine_t *engine, size_t argc)
  __attribute__((malloc, nonnull));

const mu_sequence_expr_t *sequence_expr_activate(mu_sequence_expr_t *expr)
  __attribute__((nonnull));

mu_datatype_stmt_t *datatype_stmt_allocate(mu_engine_t *engine, size_t argc)
  __attribute__((malloc, nonnull));

const mu_datatype_stmt_t *datatype_stmt_activate(
    mu_datatype_stmt_t *stmt, const mu_name_t *name)
  __attribute__((nonnull));

#endif /* MU_STATOR_NODE_I */
