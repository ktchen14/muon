#define MUON_ENGINE_MODULE

#include "node.h"

#include "common.h"
#include "name.h"

#include "../common.h"

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/// @internal Return the mutable engine of the @a node
static inline MuonEngine *unlock_engine(MuonNode *node) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
  return (MuonEngine *) node->engine;
#pragma GCC diagnostic pop
}

/// @internal Allocate a node of size @a size in the @a engine
[[gnu::malloc, gnu::nonnull]]
static inline void *node_allocate(MuonEngine *engine, size_t size) {
  if (rare((size = struct_size(NodeHeader, data, size)) == 0))
    return errno = ENOMEM, NULL;

  NodeHeader *header;
  if ((header = engine_allocate(engine, size)) == NULL)
    return NULL;
  *header = (NodeHeader) {0};

  return header->data;
}

/// @internal Assign the abstract @a node to the @a engine
[[gnu::nonnull, gnu::returns_nonnull]]
static inline MuonNode *assign_node(MuonEngine *engine, struct MuonNode *node) {
  Engine *internal = as_engine(engine);

  node->engine = engine;
  node->id = internal->node_number++;
  return node;
}

/* __attribute__((malloc, nonnull)) */
/* static inline MuonNode *node_create( */
/*     mu_engine_t *engine, size_t size, void *data) { */
/*   MuonNode *result; */
/*   if ((result = node_allocate(engine, size)) == NULL) */
/*     return NULL; */
/*   memcpy(result, data, size); */
/*   return assign_node(engine, result); */
/* } */

MuonAccessExpr *muon_access_expr(MuonEngine *engine, MuonName *name) {
  assert(name->engine == engine);

  struct MuonAccessExpr *result;
  if ((result = node_allocate(engine, sizeof(MuonAccessExpr))) == NULL)
    return NULL;
  *result = (MuonAccessExpr) {
    .as_expr.kind = MUON_ACCESS_EXPR, .name = name,
  };
  return assign_node(engine, &result->as_node), result;
}

MuonBooleanExpr *muon_boolean_expr(MuonEngine *engine, _Bool data) {
  struct MuonBooleanExpr *result;
  if ((result = node_allocate(engine, sizeof(MuonBooleanExpr))) == NULL)
    return NULL;
  *result = (MuonBooleanExpr) {
    .as_expr.kind = MUON_BOOLEAN_EXPR, .data = data,
  };
  return assign_node(engine, &result->as_node), result;
}

MuonCastExpr *muon_cast_expr(
    MuonEngine *engine, MuonSign *sign, MuonExpr *matter) {
  struct MuonCastExpr *result;
  if ((result = node_allocate(engine, sizeof(MuonCastExpr))) == NULL)
    return NULL;
  *result = (MuonCastExpr) {
    .as_expr.kind = MUON_CAST_EXPR, .sign = sign, .matter = matter,
  };
  return assign_node(engine, &result->as_node), result;
}

MuonIntegerExpr *muon_integer_expr(MuonEngine *engine, uint64_t data) {
  struct MuonIntegerExpr *result;
  if ((result = node_allocate(engine, sizeof(MuonIntegerExpr))) == NULL)
    return NULL;
  *result = (MuonIntegerExpr) {
    .as_expr.kind = MUON_INTEGER_EXPR, .data = data,
  };
  return assign_node(engine, &result->as_node), result;
}

MuonInvokeExpr *muon_invoke_expr(
    MuonEngine *engine, MuonExpr *operator, MuonExpr *argument) {
  assert(operator->as_node.engine == engine);
  assert(argument->as_node.engine == engine);

  struct MuonInvokeExpr *result;
  if ((result = node_allocate(engine, sizeof(MuonInvokeExpr))) == NULL)
    return NULL;
  *result = (MuonInvokeExpr) {
    .as_expr.kind = MUON_INVOKE_EXPR, .operator = operator, .argument = argument,
  };
  return assign_node(engine, &result->as_node), result;
}

MuonLambdaExpr *muon_lambda_expr(
    MuonEngine *engine, MuonView *argument, MuonExpr *matter) {
  assert(argument->as_node.engine == engine);
  assert(matter->as_node.engine == engine);

  struct MuonLambdaExpr *result;
  if ((result = node_allocate(engine, sizeof(MuonLambdaExpr))) == NULL)
    return NULL;
  *result = (MuonLambdaExpr) {
    .as_expr.kind = MUON_LAMBDA_EXPR, .argument = argument, .matter = matter,
  };
  return assign_node(engine, &result->as_node), result;
}

MuonNameExpr *muon_name_expr(MuonEngine *engine, MuonName *name) {
  assert(name->engine == engine);

  struct MuonNameExpr *result;
  if ((result = node_allocate(engine, sizeof(MuonNameExpr))) == NULL)
    return NULL;
  *result = (MuonNameExpr) { .as_expr.kind = MUON_NAME_EXPR, .name = name };
  return assign_node(engine, &result->as_node), result;
}

MuonNativeExpr *muon_native_expr(
    MuonEngine *engine, MuonName *name) {
  assert(name->engine == engine);

  struct MuonNativeExpr *result;
  if ((result = node_allocate(engine, sizeof(MuonNativeExpr))) == NULL)
    return NULL;
  *result = (MuonNativeExpr) {
    .as_expr.kind = MUON_NATIVE_EXPR, .name = name,
  };
  return assign_node(engine, &result->as_node), result;
}

MuonExprMember *muon_expr_member(
    MuonEngine *engine, MuonName *name, MuonExpr *expr) {
  assert(name == NULL || name->engine == engine);
  assert(expr->as_node.engine == engine);

  struct MuonExprMember *result;
  if ((result = node_allocate(engine, sizeof(MuonExprMember))) == NULL)
    return NULL;
  *result = (MuonExprMember) {
    .as_node.kind = MUON_EXPR_MEMBER_NODE, .name = name, .expr = expr,
  };
  return assign_node(engine, &result->as_node), result;
}

MuonRecordExpr *muon_record_expr(
    MuonEngine *engine, size_t argc, MuonExprMember *argv[]) {
  assert(argc == 0 || argv != NULL);

  struct MuonRecordExpr *result;
  if ((result = record_expr_allocate(engine, argc)) == NULL)
    return NULL;
  for (size_t i = 0; i < argc; i++)
    result->argv[i] = argv[i];
  return record_expr_activate(result);
}

MuonSwitchCase *muon_switch_case(
    MuonEngine *engine, MuonName *name, MuonExpr *expr) {
  assert(name->engine == engine);
  assert(expr->as_node.engine == engine);

  struct MuonSwitchCase *result;
  if ((result = node_allocate(engine, sizeof(MuonSwitchCase))) == NULL)
    return NULL;
  *result = (MuonSwitchCase) {
    .as_node.kind = MUON_SWITCH_CASE_NODE, .name = name, .expr = expr,
  };
  return assign_node(engine, &result->as_node), result;
}

MuonSwitchExpr *muon_switch_expr(
    MuonEngine *engine, size_t argc, MuonSwitchCase *const argv[argc]) {
  struct MuonSwitchExpr *result;
  if ((result = switch_expr_allocate(engine, argc)) == NULL)
    return NULL;
  for (size_t i = 0; i < argc; i++)
    result->argv[i] = argv[i];
  return switch_expr_activate(result);
}

MuonSequenceExpr *muon_sequence_expr(
    MuonEngine *engine, size_t argc, MuonStmt *const argv[argc]) {
  struct MuonSequenceExpr *result;
  if ((result = sequence_expr_allocate(engine, argc)) == NULL)
    return NULL;
  for (size_t i = 0; i < argc; i++)
    result->argv[i] = argv[i];
  return sequence_expr_activate(result);
}

MuonVectorExpr *muon_vector_expr(
    MuonEngine *engine, size_t argc, MuonExpr *const argv[]) {
  assert(argc == 0 || argv != NULL);

  for (size_t i = 0; i < argc; i++) {
    assert(argv[i] != NULL);
    assert(argv[i]->as_node.engine == engine);
  }

  size_t size;
  if (rare((size = struct_size(MuonVectorExpr, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  struct MuonVectorExpr *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (MuonVectorExpr) {
    .as_expr.kind = MUON_VECTOR_EXPR, .argc = argc,
  };

  for (size_t i = 0; i < argc; i++)
    result->argv[i] = argv[i];

  return assign_node(engine, &result->as_node), result;
}

struct MuonRecordExpr *record_expr_allocate(MuonEngine *engine, size_t argc) {
  size_t size;
  if (rare((size = struct_size(MuonRecordExpr, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  struct MuonRecordExpr *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (MuonRecordExpr) { .as_node.engine = engine, .argc = argc };
  return result;
}

MuonRecordExpr *record_expr_activate(struct MuonRecordExpr *expr) {
  MuonEngine *engine = unlock_engine(&expr->as_node);

  for (size_t i = 0; i < expr->argc; i++) {
    assert(expr->argv[i] != NULL);
    assert(expr->argv[i]->as_node.engine == engine);
  }

  MuonRecordExpr source = {
    .as_expr.kind = MUON_RECORD_EXPR, .argc = expr->argc
  };
  memcpy(expr, &source, offsetof(MuonRecordExpr, argv));
  return assign_node(engine, &expr->as_node), expr;
}

struct MuonSwitchExpr *switch_expr_allocate(MuonEngine *engine, size_t argc) {
  size_t size;
  if (rare((size = struct_size(MuonSwitchExpr, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  struct MuonSwitchExpr *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (MuonSwitchExpr) { .as_node.engine = engine, .argc = argc };
  return result;
}

MuonSwitchExpr *switch_expr_activate(struct MuonSwitchExpr *expr) {
  MuonEngine *engine = unlock_engine(&expr->as_node);

  for (size_t i = 0; i < expr->argc; i++) {
    assert(expr->argv[i] != NULL);
    assert(expr->argv[i]->as_node.engine == engine);
  }

  MuonSwitchExpr source = {
    .as_expr.kind = MUON_SWITCH_EXPR, .argc = expr->argc
  };
  memcpy(expr, &source, offsetof(MuonSwitchExpr, argv));
  return assign_node(engine, &expr->as_node), expr;
}

struct MuonSequenceExpr *sequence_expr_allocate(
    MuonEngine *engine, size_t argc) {
  size_t size;
  if (rare((size = struct_size(MuonSequenceExpr, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  struct MuonSequenceExpr *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (MuonSequenceExpr) { .as_node.engine = engine, .argc = argc };
  return result;
}

MuonSequenceExpr *sequence_expr_activate(struct MuonSequenceExpr *expr) {
  MuonEngine *engine = unlock_engine(&expr->as_node);

  for (size_t i = 0; i < expr->argc; i++) {
    assert(expr->argv[i] != NULL);
    assert(expr->argv[i]->as_node.engine == engine);
  }

  MuonSequenceExpr source = {
    .as_expr.kind = MUON_SEQUENCE_EXPR, .argc = expr->argc
  };
  memcpy(expr, &source, offsetof(MuonSequenceExpr, argv));
  return assign_node(engine, &expr->as_node), expr;
}

MuonBooleanSign *muon_boolean_sign(MuonEngine *engine) {
  struct MuonBooleanSign *result;
  if ((result = node_allocate(engine, sizeof(MuonBooleanSign))) == NULL)
    return NULL;
  *result = (MuonBooleanSign) { .as_sign.kind = MUON_BOOLEAN_SIGN };
  return assign_node(engine, &result->as_node), result;
}

MuonIntegerSign *muon_integer_sign(MuonEngine *engine) {
  struct MuonIntegerSign *result;
  if ((result = node_allocate(engine, sizeof(MuonIntegerSign))) == NULL)
    return NULL;
  *result = (MuonIntegerSign) { .as_sign.kind = MUON_INTEGER_SIGN };
  return assign_node(engine, &result->as_node), result;
}

MuonLambdaSign *muon_lambda_sign(
    MuonEngine *engine, MuonSign *argument, MuonSign *output) {
  assert(argument->as_node.engine == engine);
  assert(output->as_node.engine == engine);

  struct MuonLambdaSign *result;
  if ((result = node_allocate(engine, sizeof(MuonLambdaSign))) == NULL)
    return NULL;
  *result = (MuonLambdaSign) {
    .as_sign.kind = MUON_LAMBDA_SIGN, .argument = argument, .output = output,
  };
  return assign_node(engine, &result->as_node), result;
}

MuonNameSign *muon_name_sign(MuonEngine *engine, MuonName *name) {
  assert(name->engine == engine);

  struct MuonNameSign *result;
  if ((result = node_allocate(engine, sizeof(MuonNameSign))) == NULL)
    return NULL;
  *result = (MuonNameSign) { .as_sign.kind = MUON_NAME_SIGN, .name = name };
  return assign_node(engine, &result->as_node), result;
}

MuonRecordSign *muon_record_sign(
    MuonEngine *engine, size_t argc, const MuonSignMember argv[]) {
  assert(argc == 0 || argv != NULL);

  for (size_t i = 0; i < argc; i++) {
    MuonName *member_name = argv[i].name;
    MuonSign *member_sign = argv[i].sign;
    assert(member_name == NULL || member_name->engine == engine);
    assert(member_sign != NULL);
    assert(member_sign->as_node.engine == engine);
  }

  size_t size;
  if (rare((size = struct_size(MuonRecordSign, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  struct MuonRecordSign *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (MuonRecordSign) {
    .as_sign.kind = MUON_RECORD_SIGN, .argc = argc,
  };

  for (size_t i = 0; i < argc; i++)
    result->argv[i] = argv[i];

  return assign_node(engine, &result->as_node), result;
}

MuonVectorSign *muon_vector_sign(MuonEngine *engine, MuonSign *matter) {
  assert(matter->as_node.engine == engine);

  struct MuonVectorSign *result;
  if ((result = node_allocate(engine, sizeof(MuonVectorSign))) == NULL)
    return NULL;
  *result = (MuonVectorSign) {
    .as_sign.kind = MUON_VECTOR_SIGN, .matter = matter,
  };
  return assign_node(engine, &result->as_node), result;
}

MuonCoercionStmt *muon_coercion_stmt(
    MuonEngine *engine, MuonSign *source, MuonSign *target, MuonExpr *expr) {
  assert(source->as_node.engine == engine);
  assert(target->as_node.engine == engine);
  assert(expr->as_node.engine == engine);

  struct MuonCoercionStmt *result;
  if ((result = node_allocate(engine, sizeof(MuonCoercionStmt))) == NULL)
    return NULL;
  *result = (MuonCoercionStmt) {
    .as_stmt.kind = MUON_COERCION_STMT,
    .source = source,
    .target = target,
    .expr = expr,
  };
  return assign_node(engine, &result->as_node), result;
}

MuonDatatypeOption *muon_datatype_option(MuonEngine *engine, MuonName *name) {
  assert(name->engine == engine);

  struct MuonDatatypeOption *result;
  if ((result = node_allocate(engine, sizeof(MuonDatatypeOption))) == NULL)
    return NULL;
  *result = (MuonDatatypeOption) {
    .as_node.kind = MUON_DATATYPE_OPTION_NODE, .name = name,
  };
  return assign_node(engine, &result->as_node), result;
}

MuonDatatypeStmt *muon_datatype_stmt(
    MuonEngine *engine,
    MuonName *name,
    size_t argc,
    MuonDatatypeOption *argv[/* argc */]) {
  struct MuonDatatypeStmt *result;
  if ((result = datatype_stmt_allocate(engine, argc)) == NULL)
    return NULL;
  for (size_t i = 0; i < argc; i++)
    result->argv[i] = argv[i];
  return datatype_stmt_activate(result, name);
}

MuonDefineStmt *muon_define_stmt(
    MuonEngine *engine, MuonName *name, MuonExpr *expr) {
  assert(name->engine == engine);
  assert(expr->as_node.engine == engine);

  struct MuonDefineStmt *result;
  if ((result = node_allocate(engine, sizeof(MuonDefineStmt))) == NULL)
    return NULL;
  *result = (MuonDefineStmt) {
    .as_stmt.kind = MUON_DEFINE_STMT, .name = name, .expr = expr,
  };
  return assign_node(engine, &result->as_node), result;
}

struct MuonDatatypeStmt *datatype_stmt_allocate(
    MuonEngine *engine, size_t argc) {
  size_t size;
  if (rare((size = struct_size(MuonDatatypeStmt, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  struct MuonDatatypeStmt *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (MuonDatatypeStmt) { .as_node.engine = engine, .argc = argc };
  return result;
}

MuonDatatypeStmt *datatype_stmt_activate(
    struct MuonDatatypeStmt *stmt, MuonName *name) {
  MuonEngine *engine = unlock_engine(&stmt->as_node);

  assert(name->engine == engine);

  for (size_t i = 0; i < stmt->argc; i++) {
    assert(stmt->argv[i] != NULL);
    assert(stmt->argv[i]->as_node.engine == engine);
  }

  MuonDatatypeStmt source = {
    .as_stmt.kind = MUON_DATATYPE_STMT, .name = name, .argc = stmt->argc,
  };
  memcpy(stmt, &source, offsetof(MuonDatatypeStmt, argv));
  return assign_node(engine, &stmt->as_node), stmt;
}

MuonViewMember *muon_view_member(
    MuonEngine *engine, MuonName *name, MuonView *view) {
  assert(name == NULL || name->engine == engine);
  assert(view->as_node.engine == engine);

  struct MuonViewMember *result;
  if ((result = node_allocate(engine, sizeof(MuonViewMember))) == NULL)
    return NULL;
  *result = (MuonViewMember) {
    .as_node.kind = MUON_EXPR_MEMBER_NODE, .name = name, .view = view,
    .announce_length = node_announce_length(&view->as_node),
  };
  return assign_node(engine, &result->as_node), result;
}

MuonRecordView *muon_record_view(
    MuonEngine *engine, size_t argc, MuonViewMember *argv[]) {
  assert(argc == 0 || argv != NULL);

  struct MuonRecordView *result;
  if ((result = record_view_allocate(engine, argc)) == NULL)
    return NULL;
  for (size_t i = 0; i < argc; i++)
    result->argv[i] = argv[i];
  return record_view_activate(result);
}

MuonVariableView *muon_variable_view(MuonEngine *engine, MuonName *name) {
  struct MuonVariableView *result;
  if ((result = node_allocate(engine, sizeof(MuonVariableView))) == NULL)
    return NULL;
  *result = (MuonVariableView) {
    .as_view.kind = MUON_VARIABLE_VIEW, .name = name,
  };
  return assign_node(engine, &result->as_node), result;
}

struct MuonRecordView *record_view_allocate(MuonEngine *engine, size_t argc) {
  size_t size;
  if (rare((size = struct_size(MuonRecordView, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  struct MuonRecordView *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (MuonRecordView) { .as_node.engine = engine, .argc = argc };
  return result;
}

MuonRecordView *record_view_activate(struct MuonRecordView *view) {
  MuonEngine *engine = unlock_engine(&view->as_node);

  for (size_t i = 0; i < view->argc; i++) {
    assert(view->argv[i] != NULL);
    assert(view->argv[i]->as_node.engine == engine);
  }

  size_t announce_length = 0;
  for (size_t i = 0; i < view->argc; i++) {
    size_t n = node_announce_length(&view->argv[i]->as_node);
    if (rare(__builtin_add_overflow(announce_length, n, &announce_length)))
      return NULL;
  }

  MuonRecordView source = {
    .as_view.kind = MUON_RECORD_VIEW, .argc = view->argc,
    .announce_length = announce_length,
  };
  memcpy(view, &source, offsetof(MuonRecordView, argv));
  return assign_node(engine, &view->as_node), view;
}

MuonScript *muon_script(
    MuonEngine *engine, size_t argc, MuonStmt *argv[/* argc */]) {
  assert(argc == 0 || argv != NULL);

  struct MuonScript *result;
  if ((result = script_allocate(engine, argc)) == NULL)
    return NULL;
  for (size_t i = 0; i < argc; i++)
    result->argv[i] = argv[i];
  return script_activate(engine, result);
}

struct MuonScript *script_allocate(MuonEngine *engine, size_t argc) {
  size_t size;
  if (rare((size = struct_size(MuonScript, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  struct MuonScript *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (MuonScript) { .as_node.engine = engine, .argc = argc };
  return result;
}

MuonScript *script_activate(MuonEngine *engine, struct MuonScript *script) {
  assert(engine == script->as_node.engine);
  for (size_t i = 0; i < script->argc; i++) {
    assert(script->argv[i] != NULL);
    assert(script->argv[i]->as_node.engine == engine);
  }

  MuonScript source = {
    .as_node.kind = MUON_SCRIPT, .argc = script->argc,
  };
  memcpy(script, &source, offsetof(MuonScript, argv));
  return assign_node(engine, &script->as_node), script;
}

#include "../inductor.h"

[[gnu::nonnull]]
static inline void debug_node_type(MuonNode *node) {
  if (debug_induce == NULL)
    return;

  MuonType *type;
  if ((type = node_type(debug_induce, node)) == NULL)
    return;

  debug(" ∷ ");
  type_debug(type, 1);
  debug(" #%zu", type->id);
}

[[gnu::nonnull]]
static inline int debug_node_coercion(MuonNode *node) {
  if (debug_induce == NULL)
    return debug_indent;

  MuonCoercion *coercion;
  if ((coercion = node_coercion(debug_induce, node)) == NULL)
    return debug_indent;

  if (coercion->kind == MU_ID_COERCION)
    return debug_indent;

  debug("%*s", debug_indent, "");
  mu_coercion_debug(coercion);
  debug(" ∷ ");
  type_debug(coercion->target, 0);
  debug(" #%zu", coercion->target->id);

  debug("\n");

  return debug_indent + 2;
}

/// Emit debugging information on the abstract @a node to the debug stream
void muon_node_debug(MuonNode *node) {  // NOLINT(misc-no-recursion)
  // Kind -> Text, e.g. [MUON_ACCESS_EXPR_NODE] = "AccessExpr"
  static const char *const KIND_TEXT[] = {
#define MUON_EMIT(Title, lower, UPPER) [MUON_##UPPER##_NODE] = #Title,
    MUON_EACH_NODE_STEM(MUON_EMIT)
#undef MUON_EMIT
  };
  const char *kind = KIND_TEXT[node->kind];

  int indent = debug_indent;
  debug_indent = debug_node_coercion(node);

  debug("%*s", debug_indent, "");
  debug(PRIsKIND, DEBUG_NODE_KIND(kind));

  switch ON_ABSTRACT_OBJECT(node) {
    case IS_CONCRETE_NODE(MuonAccessExpr *access_expr)
      debug("(name = " PRIsNAME ")", DEBUG_NAME(access_expr->name));
      break;

    case IS_CONCRETE_NODE(MuonBooleanExpr *boolean_expr)
      debug("(data = %s)", boolean_expr->data ? "true" : "false");
      break;

    case IS_CONCRETE_NODE(MuonIntegerExpr *integer_expr)
      debug("(data = %" PRIu64 ")", integer_expr->data);
      break;

    case IS_CONCRETE_NODE(MuonNameExpr *name_expr)
      debug("(name = " PRIsNAME ")", DEBUG_NAME(name_expr->name));
      break;

    case IS_CONCRETE_NODE(MuonNativeExpr *native_expr)
      debug("(name = " PRIsNAME ")", DEBUG_NAME(native_expr->name));
      break;

    case IS_CONCRETE_NODE(MuonExprMember *expr_member)
      if (expr_member->name != NULL)
        debug("(name = " PRIsNAME ")", DEBUG_NAME(expr_member->name));
      break;

    case IS_CONCRETE_NODE(MuonSwitchCase *switch_case)
      debug("(name = " PRIsNAME ")", DEBUG_NAME(switch_case->name));
      break;

    case IS_CONCRETE_NODE(MuonNameSign *name_sign)
      debug("(name = " PRIsNAME ")", DEBUG_NAME(name_sign->name));
      break;

    case IS_CONCRETE_NODE(MuonDatatypeOption *datatype_option)
      debug("(name = " PRIsNAME ")", DEBUG_NAME(datatype_option->name));
      break;

    case IS_CONCRETE_NODE(MuonDatatypeStmt *datatype_stmt)
      debug("(name = " PRIsNAME ")", DEBUG_NAME(datatype_stmt->name));
      break;

    case IS_CONCRETE_NODE(MuonDefineStmt *define_stmt)
      debug("(name = " PRIsNAME ")", DEBUG_NAME(define_stmt->name));
      break;

    case IS_CONCRETE_NODE(MuonViewMember *view_member)
      if (view_member->name != NULL)
        debug("(name = " PRIsNAME ")", DEBUG_NAME(view_member->name));
      break;

    case IS_CONCRETE_NODE(MuonVariableView *variable_view)
      debug("(name = " PRIsNAME ")", DEBUG_NAME(variable_view->name));
      break;

    default: break;
  }

  debug(" #%zu", node->id);

  debug_node_type(node);
  debug("\n");

  WITH_DEBUG_INDENT() {
    size_t i = 0;
    for (MuonNode *next; (next = node_at(node, i)) != NULL; i++)
      muon_node_debug(next);
  }

  debug_indent = indent;
}
