#ifndef MUON_ENGINE_NODE_I
#define MUON_ENGINE_NODE_I

#include <muon/engine/node.h>  // IWYU pragma: export

#include "common.h"
#include "name.h"

#include "../common.h"

#include <assert.h>
#include <stddef.h>

typedef struct {
  MuonNode *anterior;
  size_t i;
} NodeCursor;

typedef struct {
  NodeCursor cursor;
  _Alignas(union {
#define MUON_EMIT(Title, lower, U) Muon##Title lower;
    MUON_EACH_NODE_STEM(MUON_EMIT)
#undef MUON_EMIT
  }) char data[];
} NodeHeader;

/// Return the cursor attached to the @a node
__attribute__((const, nonnull, returns_nonnull))
static inline NodeCursor *node_cursor(MuonNode *node) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wcast-qual"
  NodeHeader *header = (NodeHeader *) ((char *) node -
      offsetof(NodeHeader, data));
#pragma GCC diagnostic pop
  return &header->cursor;
}

/// Continue into the node
static inline MuonNode *node_continue(MuonNode *node, MuonNode *next) {
  NodeCursor *cursor = node_cursor(next);
  assert(cursor->anterior == NULL && cursor->i == 0);
  cursor->anterior = node;
  return next;
}

/// Return from the node
__attribute__((nonnull))
static inline MuonNode *node_return(MuonNode *node) {
  NodeCursor *cursor = node_cursor(node);
  MuonNode *anterior = cursor->anterior;
  *cursor = (NodeCursor) {0};
  return anterior;
}

/**
 * @brief Used to emit a case label within a switch ON_ABSTRACT_OBJECT()
 *
 * @c ... must be a declaration of a variable with a concrete node type.
 *
 * TODO
 *
 * @par Example
 * @code{.c}
 *   MuonNode *node = ...
 *
 *   switch ON_ABSTRACT_OBJECT(node) {
 *     case IS_CONCRETE_NODE(MuonAccessExpr *access_expr)
 *       return access_expr->name;
 *
 *     case IS_CONCRETE_NODE(MuonNameExpr *name_expr)
 *       return name_expr->name;
 *
 *     ...
 *   }
 * @endcode
 *
 * The behavior is undefined if:
 * - This is used except after the case keyword within a switch statement
 *   controlled by ON_ABSTRACT_OBJECT()
 * - @c ... isn't a declaration of a variable with the type of a concrete node
 */
#define IS_CONCRETE_NODE(...) \
  _Pragma("GCC diagnostic push") \
  _Pragma("GCC diagnostic ignored \"-Wunknown-warning-option\"") \
  _Pragma("GCC diagnostic ignored \"-Wdefault-const-init-var-unsafe\"") \
  MUON_NODE_ENUMERATOR_MINIMUM( \
    __extension__ ({ __attribute__((unused)) __VA_ARGS__, _; &_; }) \
  ) ... \
  MUON_NODE_ENUMERATOR_MAXIMUM( \
    __extension__ ({ __attribute__((unused)) __VA_ARGS__, _; &_; }) \
  ) \
  _Pragma("GCC diagnostic pop"): __VA_ARGS__ = _object;

/// Return the <em>i</em>th node in the abstract @a node
static inline MuonNode *node_at(MuonNode *node, size_t i) {
  switch ON_ABSTRACT_OBJECT(node) {
    case MUON_ACCESS_EXPR:
    case MUON_BOOLEAN_EXPR:
    case MUON_INTEGER_EXPR:
    case MUON_NAME_EXPR:
    case MUON_NATIVE_EXPR:
    case MUON_BOOLEAN_SIGN:
    case MUON_INTEGER_SIGN:
    case MUON_NAME_SIGN:
    case MUON_DATATYPE_OPTION:
    case MUON_VARIABLE_VIEW: return NULL;

    case IS_CONCRETE_NODE(MuonCastExpr *cast_expr)
      return (MuonNode *[]) {
        &cast_expr->sign->as_node, &cast_expr->matter->as_node, NULL
      }[i];

    case IS_CONCRETE_NODE(MuonInvokeExpr *invoke_expr)
      return (MuonNode *[]) {
        &invoke_expr->operator->as_node, &invoke_expr->argument->as_node, NULL
      }[i];

    case IS_CONCRETE_NODE(MuonLambdaExpr *lambda_expr)
      return (MuonNode *[]) {
        &lambda_expr->argument->as_node, &lambda_expr->matter->as_node, NULL
      }[i];

    case IS_CONCRETE_NODE(MuonExprMember *expr_member)
      return (MuonNode *[]) {&expr_member->expr->as_node, NULL}[i];

    case IS_CONCRETE_NODE(MuonRecordExpr *record_expr)
      return i < record_expr->argc ? &record_expr->argv[i]->as_node : NULL;

    case IS_CONCRETE_NODE(MuonSequenceExpr *sequence_expr)
      return i < sequence_expr->argc ? &sequence_expr->argv[i]->as_node : NULL;

    case IS_CONCRETE_NODE(MuonSwitchCase *switch_case)
      return (MuonNode *[]) {&switch_case->expr->as_node, NULL}[i];

    case IS_CONCRETE_NODE(MuonSwitchExpr *switch_expr)
      return i < switch_expr->argc ? &switch_expr->argv[i]->as_node : NULL;

    case IS_CONCRETE_NODE(MuonLambdaSign *lambda_sign)
      return (MuonNode *[]) {
        &lambda_sign->argument->as_node, &lambda_sign->output->as_node, NULL
      }[i];

    case IS_CONCRETE_NODE(MuonVectorExpr *vector_expr)
      return i < vector_expr->argc ? &vector_expr->argv[i]->as_node : NULL;

    case IS_CONCRETE_NODE(MuonRecordSign *record_sign)
      return i < record_sign->argc ? &record_sign->argv[i].sign->as_node : NULL;

    case IS_CONCRETE_NODE(MuonVectorSign *vector_sign)
      return (MuonNode *[]) {&vector_sign->matter->as_node, NULL}[i];

    case IS_CONCRETE_NODE(MuonCoercionStmt *coercion_stmt)
      return (MuonNode *[]) {
        &coercion_stmt->source->as_node,
        &coercion_stmt->target->as_node,
        &coercion_stmt->expr->as_node,
        NULL,
      }[i];

    case IS_CONCRETE_NODE(MuonDatatypeStmt *datatype_stmt)
      return i < datatype_stmt->argc ? &datatype_stmt->argv[i]->as_node : NULL;

    case IS_CONCRETE_NODE(MuonDefineStmt *define_stmt)
      return (MuonNode *[]) {&define_stmt->expr->as_node, NULL}[i];

    case IS_CONCRETE_NODE(MuonViewMember *view_member)
      return (MuonNode *[]) {&view_member->view->as_node, NULL}[i];

    case IS_CONCRETE_NODE(MuonRecordView *record_view)
      return i < record_view->argc ? &record_view->argv[i]->as_node : NULL;

    case IS_CONCRETE_NODE(MuonScript *script)
      return i < script->argc ? &script->argv[i]->as_node : NULL;
  }
  __builtin_unreachable();
}

/// Return the announce length of the abstract @a node
static inline size_t node_announce_length(MuonNode *node) {
  switch ON_ABSTRACT_OBJECT(node) {
    case IS_CONCRETE_NODE(MuonDatatypeStmt *datatype_stmt)
      return datatype_stmt->argc + 1;

    case MUON_DEFINE_STMT: return 1;

    case IS_CONCRETE_NODE(MuonViewMember *view_member)
      return view_member->announce_length;

    case IS_CONCRETE_NODE(MuonRecordView *record_view)
      return record_view->announce_length;

    case MUON_VARIABLE_VIEW: return 1;

    default: return 0;
  }
  __builtin_unreachable();
}

struct MuonRecordExpr *record_expr_allocate(MuonEngine *engine, size_t argc)
  __attribute__((malloc, nonnull));

MuonRecordExpr *record_expr_activate(struct MuonRecordExpr *expr)
  __attribute__((nonnull));

struct MuonSwitchExpr *switch_expr_allocate(MuonEngine *engine, size_t argc)
  __attribute__((malloc, nonnull));

MuonSwitchExpr *switch_expr_activate(struct MuonSwitchExpr *expr)
  __attribute__((nonnull));

struct MuonSequenceExpr *sequence_expr_allocate(MuonEngine *engine, size_t argc)
  __attribute__((malloc, nonnull));

MuonSequenceExpr *sequence_expr_activate(struct MuonSequenceExpr *expr)
  __attribute__((nonnull));

struct MuonDatatypeStmt *datatype_stmt_allocate(MuonEngine *engine, size_t argc)
  __attribute__((malloc, nonnull));

MuonDatatypeStmt *datatype_stmt_activate(
    struct MuonDatatypeStmt *stmt, MuonName *name)
  __attribute__((nonnull));

struct MuonRecordView *record_view_allocate(MuonEngine *engine, size_t argc)
  __attribute__((malloc, nonnull));

MuonRecordView *record_view_activate(struct MuonRecordView *view)
  __attribute__((nonnull));

struct MuonScript *script_allocate(MuonEngine *engine, size_t argc)
  __attribute__((malloc, nonnull));

MuonScript *script_activate(MuonEngine *engine, struct MuonScript *script)
  __attribute__((nonnull));

#endif /* MUON_ENGINE_NODE_I */
