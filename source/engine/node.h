#ifndef MU_ENGINE_NODE_I
#define MU_ENGINE_NODE_I

#include <muon/engine/node.h>  // IWYU pragma: export

#include "common.h"
#include "name.h"

#include "../common.h"

#include <assert.h>
#include <stddef.h>

/// @internal An enumeration over each kind of node, e.g. @c _access_expr_kind
enum {
#define MU_EMIT(lower, upper, t) _##lower##_kind = MU_##upper##_NODE,
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wcast-qual"
  node_header_t *header = (node_header_t *) (
      (char *) node - offsetof(node_header_t, data));
#pragma GCC diagnostic pop
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

#define INTERNAL_IS_CONCRETE_NODE(type, name) \
  MU_NODE_ENUMERATOR(type):; __typeof__(type) name = _object;

#define IS_CONCRETE_NODE(...) INTERNAL_IS_CONCRETE_NODE(__VA_ARGS__)

#define nominate(name) , name

/// Return the <em>i</em>th node in the abstract @a node
static inline const mu_node_t *node_at(const mu_node_t *node, size_t i) {
  switch ON_ABSTRACT_OBJECT(node) {
    case MU_ACCESS_EXPR:
    case MU_BOOLEAN_EXPR:
    case MU_INTEGER_EXPR:
    case MU_NAME_EXPR:
    case MU_NATIVE_EXPR:
    case MU_BOOLEAN_SIGN:
    case MU_INTEGER_SIGN:
    case MU_NAME_SIGN:
    case MU_DATATYPE_OPTION:
    case MU_VARIABLE_VIEW:
      return NULL;

    case IS_CONCRETE_NODE(const mu_cast_expr_t *nominate(cast_expr))
      return (const mu_node_t *[]) {
        &cast_expr->sign->as_node, &cast_expr->matter->as_node, NULL,
      }[i];

    case IS_CONCRETE_NODE(const mu_invoke_expr_t *nominate(invoke_expr))
      return (const mu_node_t *[]) {
        &invoke_expr->operator->as_node, &invoke_expr->argument->as_node, NULL,
      }[i];

    case IS_CONCRETE_NODE(const mu_lambda_expr_t *nominate(lambda_expr))
      return (const mu_node_t *[]) {
        &lambda_expr->argument->as_node, &lambda_expr->matter->as_node, NULL,
      }[i];

    case IS_CONCRETE_NODE(const mu_expr_member_t *nominate(expr_member))
      return (const mu_node_t *[]) { &expr_member->expr->as_node, NULL }[i];

    case IS_CONCRETE_NODE(const mu_record_expr_t *nominate(record_expr))
      return i < record_expr->argc ? &record_expr->argv[i]->as_node : NULL;

    case IS_CONCRETE_NODE(const mu_sequence_expr_t *nominate(sequence_expr))
      return i < sequence_expr->argc ? &sequence_expr->argv[i]->as_node : NULL;

    case IS_CONCRETE_NODE(const mu_switch_case_t *nominate(switch_case))
      return (const mu_node_t *[]) { &switch_case->expr->as_node, NULL }[i];

    case IS_CONCRETE_NODE(const mu_switch_expr_t *nominate(switch_expr))
      return i < switch_expr->argc ? &switch_expr->argv[i]->as_node : NULL;

    case IS_CONCRETE_NODE(const mu_lambda_sign_t *nominate(lambda_sign))
      return (const mu_node_t *[]) {
        &lambda_sign->argument->as_node, &lambda_sign->output->as_node, NULL,
      }[i];

    case IS_CONCRETE_NODE(const mu_vector_expr_t *nominate(vector_expr))
      return i < vector_expr->argc ? &vector_expr->argv[i]->as_node : NULL;

    case IS_CONCRETE_NODE(const mu_record_sign_t *nominate(record_sign))
      return i < record_sign->argc ? &record_sign->argv[i].sign->as_node : NULL;

    case IS_CONCRETE_NODE(const mu_vector_sign_t *nominate(vector_sign))
      return (const mu_node_t *[]) { &vector_sign->matter->as_node, NULL }[i];

    case IS_CONCRETE_NODE(const mu_coercion_stmt_t *nominate(coercion_stmt))
      return (const mu_node_t *[]) {
        &coercion_stmt->source->as_node,
        &coercion_stmt->target->as_node,
        &coercion_stmt->expr->as_node, NULL,
      }[i];

    case IS_CONCRETE_NODE(const mu_datatype_stmt_t *nominate(datatype_stmt))
      return i < datatype_stmt->argc ? &datatype_stmt->argv[i]->as_node : NULL;

    case IS_CONCRETE_NODE(const mu_define_stmt_t *nominate(define_stmt))
      return (const mu_node_t *[]) { &define_stmt->expr->as_node, NULL }[i];

    case IS_CONCRETE_NODE(const mu_view_member_t *nominate(view_member))
      return (const mu_node_t *[]) { &view_member->view->as_node, NULL }[i];

    case IS_CONCRETE_NODE(const mu_record_view_t *nominate(record_view))
      return i < record_view->argc ? &record_view->argv[i]->as_node : NULL;
  }
  __builtin_unreachable();
}

/// Return the announce length of the abstract @a node
static inline size_t node_announce_length(const mu_node_t *node) {
  switch ON_ABSTRACT_OBJECT(node) {
    case IS_KIND_OF(datatype_stmt):
      return datatype_stmt->argc + 1;

    case MU_DEFINE_STMT:
      return 1;

    case IS_KIND_OF(view_member):
      return view_member->announce_length;

    case IS_KIND_OF(record_view):
      return record_view->announce_length;

    case MU_VARIABLE_VIEW:
      return 1;

    default: return 0;
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
    mu_datatype_stmt_t *stmt, mu_name_t *name)
  __attribute__((nonnull));

mu_record_view_t *record_view_allocate(mu_engine_t *engine, size_t argc)
  __attribute__((malloc, nonnull));

const mu_record_view_t *record_view_activate(mu_record_view_t *view)
  __attribute__((nonnull));

#endif /* MU_ENGINE_NODE_I */
