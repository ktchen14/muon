#ifndef MUON_ENGINE_NODE_I
#define MUON_ENGINE_NODE_I

#include <muon/engine/node.h>

#include "common.h"

#include <assert.h>
#include <stddef.h>

typedef struct {
  union {
    /// @internal Used to traverse a node tree
    struct NodeCursor {
      MuonNode *node; size_t i; //-
    } cursor;

    /// @internal Used to assemble a node list
    struct NodeSeries {
      MuonNode *next; size_t n; //-
    } series;
  };

  _Alignas(union {
#define MUON_EMIT(Title, lower, U) Muon##Title lower;
    MUON_EACH_NODE_STEM(MUON_EMIT)
#undef MUON_EMIT
  }) char node[];
} NodeHeader;

/// Return the cursor of the @a node
MUON_HINT(const, nonnull, returns_nonnull)
static inline struct NodeCursor *node_cursor(MuonNode *node) {
  const size_t offset = offsetof(NodeHeader, node);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wcast-qual"
  NodeHeader *header = (NodeHeader *) ((char *) node - offset);
#pragma GCC diagnostic pop
  return &header->cursor;
}

/// Return the series of the @a node
MUON_HINT(const, nonnull, returns_nonnull)
static inline struct NodeSeries *node_series(MuonNode *node) {
  const size_t offset = offsetof(NodeHeader, node);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wcast-qual"
  NodeHeader *header = (NodeHeader *) ((char *) node - offset);
#pragma GCC diagnostic pop
  return &header->series;
}

/// Continue into the node
MUON_HINT(nonnull(2), returns_nonnull)
static inline MuonNode *node_continue(
    MuonNode *restrict node, MuonNode *restrict next) {
  struct NodeCursor *cursor = node_cursor(next);
  assert(cursor->node == NULL && cursor->i == 0);
  return cursor->node = node, next;
}

/// Return from the node
MUON_HINT(nonnull)
static inline MuonNode *node_return(MuonNode *node) {
  struct NodeCursor *cursor = node_cursor(node);
  node = cursor->node;
  return *cursor = (struct NodeCursor) {}, node;
}

MUON_HINT(nonnull(2), returns_nonnull)
static inline MuonNode *node_attach(
    MuonNode *restrict node, MuonNode *restrict next) {
  struct NodeSeries *series = node_series(next);
  assert(series->next == NULL && series->n == 0);

  if (node == NULL)
    return series->next = next;

  series = node_series(node);
  assert(series->next != NULL);
  *node_series(next) = *series;
  return series->next = next;
}

#define node_attach(node, next) ((typeof(next)) node_attach( \
  object_member((typeof(next)) {node}, offsetof(typeof(*(next)), as_node)), \
  &(next)->as_node))

MUON_HINT(nonnull) static inline MuonNode *node_detach(MuonNode *node) {
  MuonNode *next = node_series(node)->next;
  assert(next != NULL);
  struct NodeSeries *series = node_series(next);
  node_series(node)->next = series->next;
  return *series = (struct NodeSeries) {}, next;
}

#define node_detach(node) ((typeof(node)) node_detach(&(node)->as_node))

/// @internal Used in ON_ABSTRACT_NODE()
static _Thread_local const void *abstract_node;

/// Used with IS_CONCRETE_NODE() to switch on the tag of the abstract @a node
#define ON_ABSTRACT_NODE(node) ((typeof(node)) {abstract_node = (node)}->tag)

/// Emit a case within a switch ON_ABSTRACT_NODE()
#define IS_CONCRETE_NODE(...) \
  MUON_NODE_TAG(typeof((struct { __VA_ARGS__, *_; }) {}._)): \
    __VA_ARGS__ = abstract_node;

/// Return the <em>i</em>th node in the abstract @a node
static inline MuonNode *node_at(MuonNode *node, size_t i) {
  switch ON_ABSTRACT_NODE(node) {
    case MUON_ACCESS_EXPR:
    case MUON_BOOLEAN_EXPR:
    case MUON_INTEGER_EXPR:
    case MUON_NAME_EXPR:
    case MUON_EXPR_IMPORT:
    case MUON_BOOLEAN_SIGN:
    case MUON_INTEGER_SIGN:
    case MUON_NAME_SIGN:
    case MUON_VARIABLE_SIGN:
    case MUON_DATATYPE_OPTION:
    case MUON_VARIABLE_VIEW:
      return NULL;

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

    case IS_CONCRETE_NODE(MuonSchemeExpr *scheme_expr)
      return (MuonNode *[]) {&scheme_expr->expr->as_node, NULL}[i];

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
  unreachable();
}

/// Return the announce length of the abstract @a node
static inline size_t node_announce_length(MuonNode *node) {
  switch ON_ABSTRACT_NODE(node) {
    case IS_CONCRETE_NODE(MuonDatatypeStmt *datatype_stmt)
      return datatype_stmt->argc + 1;

    case MUON_DEFINE_STMT:
      return 1;

    case IS_CONCRETE_NODE(MuonViewMember *view_member)
      return view_member->announce_length;

    case IS_CONCRETE_NODE(MuonRecordView *record_view)
      return record_view->announce_length;

    case MUON_VARIABLE_VIEW:
      return 1;

    default:
      return 0;
  }
  unreachable();
}

struct MuonVectorExpr *vector_expr_allocate(MuonEngine *engine, size_t argc)
  MUON_HINT_SUFFIX(nonnull);

MuonVectorExpr *vector_expr_activate(struct MuonVectorExpr *expr)
  MUON_HINT_SUFFIX(nonnull);

struct MuonRecordExpr *record_expr_allocate(MuonEngine *engine, size_t argc)
  MUON_HINT_SUFFIX(nonnull);

MuonRecordExpr *record_expr_activate(struct MuonRecordExpr *expr)
  MUON_HINT_SUFFIX(nonnull);

struct MuonSwitchExpr *switch_expr_allocate(MuonEngine *engine, size_t argc)
  MUON_HINT_SUFFIX(nonnull);

MuonSwitchExpr *switch_expr_activate(struct MuonSwitchExpr *expr)
  MUON_HINT_SUFFIX(nonnull);

struct MuonSequenceExpr *sequence_expr_allocate(MuonEngine *engine, size_t argc)
  MUON_HINT_SUFFIX(nonnull);

MuonSequenceExpr *sequence_expr_activate(struct MuonSequenceExpr *expr)
  MUON_HINT_SUFFIX(nonnull);

struct MuonDatatypeStmt *datatype_stmt_allocate(MuonEngine *engine, size_t argc)
  MUON_HINT_SUFFIX(nonnull);

MuonDatatypeStmt *datatype_stmt_activate(
    struct MuonDatatypeStmt *stmt, MuonName *name)
  MUON_HINT_SUFFIX(nonnull);

struct MuonRecordView *record_view_allocate(MuonEngine *engine, size_t argc)
  MUON_HINT_SUFFIX(nonnull);

MuonRecordView *record_view_activate(struct MuonRecordView *view)
  MUON_HINT_SUFFIX(nonnull);

struct MuonScript *script_allocate(MuonEngine *engine, size_t argc)
  MUON_HINT_SUFFIX(nonnull);

MuonScript *script_activate(MuonEngine *engine, struct MuonScript *script)
  MUON_HINT_SUFFIX(nonnull);

#endif /* MUON_ENGINE_NODE_I */
