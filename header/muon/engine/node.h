#ifndef MUON_ENGINE_NODE_H
#define MUON_ENGINE_NODE_H

#include "common.h"
#include "name.h"

#include <stddef.h>
#include <stdint.h>

/// Expands to emit(Title, lower, UPPER, ...) for each kind of expr
#define MUON_EACH_EXPR_STEM(emit, ...) \
  emit(AccessExpr, access_expr, ACCESS_EXPR __VA_OPT__(,) __VA_ARGS__) \
  emit(BooleanExpr, boolean_expr, BOOLEAN_EXPR __VA_OPT__(,) __VA_ARGS__) \
  emit(CastExpr, cast_expr, CAST_EXPR __VA_OPT__(,) __VA_ARGS__) \
  emit(IntegerExpr, integer_expr, INTEGER_EXPR __VA_OPT__(,) __VA_ARGS__) \
  emit(InvokeExpr, invoke_expr, INVOKE_EXPR __VA_OPT__(,) __VA_ARGS__) \
  emit(LambdaExpr, lambda_expr, LAMBDA_EXPR __VA_OPT__(,) __VA_ARGS__) \
  emit(NameExpr, name_expr, NAME_EXPR __VA_OPT__(,) __VA_ARGS__) \
  emit(NativeExpr, native_expr, NATIVE_EXPR __VA_OPT__(,) __VA_ARGS__) \
  emit(RecordExpr, record_expr, RECORD_EXPR __VA_OPT__(,) __VA_ARGS__) \
  emit(SequenceExpr, sequence_expr, SEQUENCE_EXPR __VA_OPT__(,) __VA_ARGS__) \
  emit(SwitchExpr, switch_expr, SWITCH_EXPR __VA_OPT__(,) __VA_ARGS__) \
  emit(VectorExpr, vector_expr, VECTOR_EXPR __VA_OPT__(,) __VA_ARGS__)

/// Expands to emit(Title, lower, UPPER, ...) for each kind of sign
#define MUON_EACH_SIGN_STEM(emit, ...) \
  emit(BooleanSign, boolean_sign, BOOLEAN_SIGN __VA_OPT__(,) __VA_ARGS__) \
  emit(IntegerSign, integer_sign, INTEGER_SIGN __VA_OPT__(,) __VA_ARGS__) \
  emit(LambdaSign, lambda_sign, LAMBDA_SIGN __VA_OPT__(,) __VA_ARGS__) \
  emit(NameSign, name_sign, NAME_SIGN __VA_OPT__(,) __VA_ARGS__) \
  emit(RecordSign, record_sign, RECORD_SIGN __VA_OPT__(,) __VA_ARGS__) \
  emit(VectorSign, vector_sign, VECTOR_SIGN __VA_OPT__(,) __VA_ARGS__)

/// Expands to emit(Title, lower, UPPER, ...) for each kind of stmt
#define MUON_EACH_STMT_STEM(emit, ...) \
  emit(CoercionStmt, coercion_stmt, COERCION_STMT __VA_OPT__(,) __VA_ARGS__) \
  emit(DatatypeStmt, datatype_stmt, DATATYPE_STMT __VA_OPT__(,) __VA_ARGS__) \
  emit(DefineStmt, define_stmt, DEFINE_STMT __VA_OPT__(,) __VA_ARGS__)

/// Expands to emit(Title, lower, UPPER, ...) for each kind of view
#define MUON_EACH_VIEW_STEM(emit, ...) \
  emit(RecordView, record_view, RECORD_VIEW __VA_OPT__(,) __VA_ARGS__) \
  emit(VariableView, variable_view, VARIABLE_VIEW __VA_OPT__(,) __VA_ARGS__)

/// Expands to emit(Title, lower, UPPER, ...) for each kind of node
#define MUON_EACH_NODE_STEM(emit, ...) \
  MUON_EACH_EXPR_STEM(emit __VA_OPT__(,) __VA_ARGS__) \
  MUON_EACH_SIGN_STEM(emit __VA_OPT__(,) __VA_ARGS__) \
  MUON_EACH_STMT_STEM(emit __VA_OPT__(,) __VA_ARGS__) \
  MUON_EACH_VIEW_STEM(emit __VA_OPT__(,) __VA_ARGS__) \
  emit(ExprMember, expr_member, EXPR_MEMBER __VA_OPT__(,) __VA_ARGS__) \
  emit(SwitchCase, switch_case, SWITCH_CASE __VA_OPT__(,) __VA_ARGS__) \
  emit(DatatypeOption, datatype_option, DATATYPE_OPTION \
    __VA_OPT__(,) __VA_ARGS__) \
  emit(ViewMember, view_member, VIEW_MEMBER __VA_OPT__(,) __VA_ARGS__) \
  emit(Script, script, SCRIPT __VA_OPT__(,) __VA_ARGS__)

#define MUON_NODE_TAG_EMIT(T, l, UPPER) MUON_##UPPER##_NODE,

/// An enumeration over each kind of node, e.g. @c MUON_ACCESS_EXPR_NODE
typedef enum {
  MUON_EACH_NODE_STEM(MUON_NODE_TAG_EMIT)

  /// Equivalent to the minimum enumerator in MuonNodeTag
  MUON_MINORANT_NODE = MUON_INDIRECT(
    MUON_TAKE, MUON_EACH_NODE_STEM(MUON_NODE_TAG_EMIT)),

  MUON_EXPR_MEMBER = MUON_EXPR_MEMBER_NODE, //-
  MUON_SWITCH_CASE = MUON_SWITCH_CASE_NODE,
  MUON_DATATYPE_OPTION = MUON_DATATYPE_OPTION_NODE,
  MUON_VIEW_MEMBER = MUON_VIEW_MEMBER_NODE,
  MUON_SCRIPT = MUON_SCRIPT_NODE,
} MuonNodeTag;

/// An enumeration over each kind of expr, e.g. @c MUON_ACCESS_EXPR
typedef enum {
#define MUON_EMIT(T, l, UPPER) MUON_##UPPER = MUON_##UPPER##_NODE,
  MUON_EACH_EXPR_STEM(MUON_EMIT)
#undef MUON_EMIT

  /// Equivalent to the minimum enumerator in MuonExprTag
  MUON_MINORANT_EXPR = MUON_INDIRECT(
    MUON_TAKE, MUON_EACH_EXPR_STEM(MUON_NODE_TAG_EMIT)),
} MuonExprTag;

/// An enumeration over each kind of sign, e.g. @c MUON_BOOLEAN_SIGN
typedef enum {
#define MUON_EMIT(T, l, UPPER) MUON_##UPPER = MUON_##UPPER##_NODE,
  MUON_EACH_SIGN_STEM(MUON_EMIT)
#undef MUON_EMIT

  /// Equivalent to the minimum enumerator in MuonSignTag
  MUON_MINORANT_SIGN = MUON_INDIRECT(
    MUON_TAKE, MUON_EACH_SIGN_STEM(MUON_NODE_TAG_EMIT)),
} MuonSignTag;

/// An enumeration over each kind of stmt, e.g. @c MUON_COERCION_STMT
typedef enum {
#define MUON_EMIT(T, l, UPPER) MUON_##UPPER = MUON_##UPPER##_NODE,
  MUON_EACH_STMT_STEM(MUON_EMIT)
#undef MUON_EMIT

  /// Equivalent to the minimum enumerator in MuonStmtTag
  MUON_MINORANT_STMT = MUON_INDIRECT(
    MUON_TAKE, MUON_EACH_STMT_STEM(MUON_NODE_TAG_EMIT)),
} MuonStmtTag;

/// An enumeration over each kind of view, e.g. @c MUON_RECORD_VIEW
typedef enum {
#define MUON_EMIT(T, l, UPPER) MUON_##UPPER = MUON_##UPPER##_NODE,
  MUON_EACH_VIEW_STEM(MUON_EMIT)
#undef MUON_EMIT

  /// Equivalent to the minimum enumerator in MuonViewTag
  MUON_MINORANT_VIEW = MUON_INDIRECT(
    MUON_TAKE, MUON_EACH_VIEW_STEM(MUON_NODE_TAG_EMIT)),
} MuonViewTag;

#undef MUON_NODE_TAG_EMIT

enum {
#define MUON_EMIT(...) + 1
  /// Number of distinct kinds of nodes
  MUON_NODE_NUMBER = MUON_EACH_NODE_STEM(MUON_EMIT),

  /// Number of distinct kinds of exprs
  MUON_EXPR_NUMBER = MUON_EACH_EXPR_STEM(MUON_EMIT),

  /// Number of distinct kinds of signs
  MUON_SIGN_NUMBER = MUON_EACH_SIGN_STEM(MUON_EMIT),

  /// Number of distinct kinds of stmts
  MUON_STMT_NUMBER = MUON_EACH_STMT_STEM(MUON_EMIT),

  /// Number of distinct kinds of views
  MUON_VIEW_NUMBER = MUON_EACH_VIEW_STEM(MUON_EMIT),
#undef MUON_EMIT
};

/**
 * @brief An abstract node
 *
 * Note that a MuonNode is a constant object; the mutable equivalent is a
 * <tt>struct MuonNode</tt>.
 */
typedef const struct MuonNode {
  MuonNodeTag kind;
  const MuonEngine *engine;
  size_t id;
} MuonNode;

/// The header that each MuonNode subtype must have
#define MUON_NODE_HEADER struct MuonNode as_node

/**
 * @brief An abstract expr
 *
 * Note that a MuonExpr is a constant object; the mutable equivalent is a
 * <tt>struct MuonExpr</tt>.
 */
typedef const struct MuonExpr {
  union { MUON_NODE_HEADER; MuonExprTag kind; }; //-
} MuonExpr;

/// The header that each MuonExpr subtype must have
#define MUON_EXPR_HEADER union { \
  struct MuonExpr as_expr; MUON_NODE_HEADER; \
}

/**
 * @brief An abstract sign
 *
 * Note that a MuonSign is a constant object; the mutable equivalent is a
 * <tt>struct MuonSign</tt>.
 */
typedef const struct MuonSign {
  union { MUON_NODE_HEADER; MuonSignTag kind; }; //-
} MuonSign;

/// The header that each MuonSign subtype must have
#define MUON_SIGN_HEADER union { \
  struct MuonSign as_sign; MUON_NODE_HEADER; \
}

/**
 * @brief An abstract stmt
 *
 * Note that a MuonStmt is a constant object; the mutable equivalent is a
 * <tt>struct MuonStmt</tt>.
 */
typedef const struct MuonStmt {
  union { MUON_NODE_HEADER; MuonStmtTag kind; }; //-
} MuonStmt;

/// The header that each MuonStmt subtype must have
#define MUON_STMT_HEADER union { \
  struct MuonStmt as_stmt; MUON_NODE_HEADER; \
}

/**
 * @brief An abstract view
 *
 * Note that a MuonView is a constant object; the mutable equivalent is a
 * <tt>struct MuonView</tt>.
 */
typedef const struct MuonView {
  union { MUON_NODE_HEADER; MuonViewTag kind; }; //-
} MuonView;

/// The header that each MuonView subtype must have
#define MUON_VIEW_HEADER union { \
  struct MuonView as_view; MUON_NODE_HEADER; \
}

typedef const struct MuonAccessExpr {
  MUON_EXPR_HEADER;
  MuonName *name;
} MuonAccessExpr;

typedef const struct MuonBooleanExpr {
  MUON_EXPR_HEADER;
  _Bool data;
} MuonBooleanExpr;

typedef const struct MuonCastExpr {
  MUON_EXPR_HEADER;
  MuonSign *sign;
  MuonExpr *matter;
} MuonCastExpr;

typedef const struct MuonIntegerExpr {
  MUON_EXPR_HEADER;
  uint64_t data;
} MuonIntegerExpr;

typedef const struct MuonInvokeExpr {
  MUON_EXPR_HEADER;
  MuonExpr *operator;
  MuonExpr *argument;
} MuonInvokeExpr;

typedef const struct MuonLambdaExpr {
  MUON_EXPR_HEADER;
  MuonView *argument;
  MuonExpr *matter;
} MuonLambdaExpr;

typedef const struct MuonNameExpr {
  MUON_EXPR_HEADER;
  MuonName *name;
} MuonNameExpr;

typedef const struct MuonNativeExpr {
  MUON_EXPR_HEADER;
  MuonName *name;
} MuonNativeExpr;

typedef const struct MuonExprMember {
  MUON_NODE_HEADER;
  MuonName *name; // optional
  MuonExpr *expr;
} MuonExprMember;

typedef const struct MuonRecordExpr {
  MUON_EXPR_HEADER;
  size_t argc;
  MuonExprMember *argv[] MUON_HINT(counted_by(argc));
} MuonRecordExpr;

typedef const struct MuonSequenceExpr {
  MUON_EXPR_HEADER;
  size_t argc;
  MuonStmt *argv[] MUON_HINT(counted_by(argc));
} MuonSequenceExpr;

typedef const struct MuonSwitchCase {
  MUON_NODE_HEADER;
  MuonName *name;
  MuonExpr *expr;
} MuonSwitchCase;

typedef const struct MuonSwitchExpr {
  MUON_EXPR_HEADER;
  size_t argc;
  MuonSwitchCase *argv[] MUON_HINT(counted_by(argc));
} MuonSwitchExpr;

typedef const struct MuonVectorExpr {
  MUON_EXPR_HEADER;
  size_t argc;
  MuonExpr *argv[] MUON_HINT(counted_by(argc));
} MuonVectorExpr;

typedef const struct MuonBooleanSign {
  MUON_SIGN_HEADER;
} MuonBooleanSign;

typedef const struct MuonIntegerSign {
  MUON_SIGN_HEADER;
} MuonIntegerSign;

typedef const struct MuonLambdaSign {
  MUON_SIGN_HEADER;
  MuonSign *argument;
  MuonSign *output;
} MuonLambdaSign;

typedef const struct MuonNameSign {
  MUON_SIGN_HEADER;
  MuonName *name;
} MuonNameSign;

typedef struct {
  MuonName *name; // optional
  MuonSign *sign;
} MuonSignMember;

typedef const struct MuonRecordSign {
  MUON_SIGN_HEADER;
  size_t argc;
  MuonSignMember argv[] MUON_HINT(counted_by(argc));
} MuonRecordSign;

typedef const struct MuonVectorSign {
  MUON_SIGN_HEADER;
  MuonSign *matter;
} MuonVectorSign;

typedef const struct MuonCoercionStmt {
  MUON_STMT_HEADER;
  MuonSign *source;
  MuonSign *target;
  MuonExpr *expr;
} MuonCoercionStmt;

typedef const struct MuonTypeNode {
  MUON_NODE_HEADER;
  MuonName *name;
  size_t argc;
  MuonName *argv[] MUON_HINT(counted_by(argc));
} MuonTypeNode;

typedef const struct MuonDatatypeOption {
  MUON_NODE_HEADER;
  MuonName *name;
} MuonDatatypeOption;

typedef const struct MuonDatatypeStmt {
  MUON_STMT_HEADER;
  MuonName *name;
  size_t argc;
  MuonDatatypeOption *argv[] MUON_HINT(counted_by(argc));
} MuonDatatypeStmt;

typedef const struct MuonDefineStmt {
  MUON_STMT_HEADER;
  MuonName *name;
  MuonExpr *expr;
} MuonDefineStmt;

typedef const struct MuonViewMember {
  MUON_NODE_HEADER;
  size_t announce_length;
  MuonName *name; // optional
  MuonView *view;
} MuonViewMember;

typedef const struct MuonRecordView {
  MUON_VIEW_HEADER;
  size_t announce_length;
  size_t argc;
  MuonViewMember *argv[] MUON_HINT(counted_by(argc));
} MuonRecordView;

typedef const struct MuonVariableView {
  MUON_VIEW_HEADER;
  MuonName *name;
} MuonVariableView;

typedef const struct MuonScript {
  MUON_NODE_HEADER;
  size_t argc;
  MuonStmt *argv[] MUON_HINT(counted_by(argc));
} MuonScript;

/// @internal Used to emit each branch in MUON_NODE_TAG()
#define MUON_NODE_TAG_EMIT(Title, l, UPPER, SUFFIX) \
  , Muon##Title *: MUON_##UPPER##SUFFIX

/// Return the enumerator indicative of the concrete @a node
#define MUON_NODE_TAG(node) _Generic((node) {} \
  MUON_EACH_NODE_STEM(MUON_NODE_TAG_EMIT, _NODE))

/// @internal Used to decide the cast result in muon_node_cast()
MUON_HINT(nonnull)
static inline MuonNode *muon_node_cast(MuonNode *node, MuonNodeTag tag) {
  return node->kind == tag ? node : NULL;
}

/**
 * @brief Downcast the @a abstract node to the <tt>typeof(concrete)</tt>
 *
 * @a abstract should have type <tt>MuonNode *</tt>. @a concrete should have the
 * type of a pointer to a const qualified concrete node. Then if
 * @a abstract is an instance of that type, it will be cast to that type and
 * returned. Otherwise, this will return @c NULL.
 *
 * Then if @a abstract is an instance of that type, it will be cast to that type
 * and returned. Otherwise, this will return @c NULL.
 *
 * @par Example:
 * @code{.c}
 *   MuonNode *abstract_node = ...;
 *
 *   MuonAccessExpr *expr;
 *   if ((expr = mu_expr_cast(abstract_node, expr)) == NULL)
 *     return ...;
 * @endcode
 *
 * The behavior is undefined if:
 * - @a abstract is @c NULL
 * - @a abstract doesn't have type <tt>MuonNode *</tt>
 * - @a concrete isn't, or doesn't have, the type of a const qualified pointer
 *   to a concrete node
 */
#define muon_node_cast(node, concrete) ( \
  (typeof(concrete)) muon_node_cast((node), MUON_NODE_TAG(typeof(concrete))) \
)

MuonAccessExpr *muon_access_expr(MuonEngine *engine, MuonName *name)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonBooleanExpr *muon_boolean_expr(MuonEngine *engine, _Bool data)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonCastExpr *muon_cast_expr(
    MuonEngine *engine, MuonSign *sign, MuonExpr *matter)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonIntegerExpr *muon_integer_expr(MuonEngine *engine, uint64_t data)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonInvokeExpr *muon_invoke_expr(
    MuonEngine *engine, MuonExpr *operator, MuonExpr *argument)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonLambdaExpr *muon_lambda_expr(
    MuonEngine *engine, MuonView *argument, MuonExpr *matter)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonNameExpr *muon_name_expr(MuonEngine *engine, MuonName *name)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonNativeExpr *muon_native_expr(MuonEngine *engine, MuonName *name)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonExprMember *muon_expr_member(
    MuonEngine *engine, MuonName *name, MuonExpr *expr)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonRecordExpr *muon_record_expr(
    MuonEngine *engine, size_t argc, MuonExprMember *argv[/* argc */])
  MUON_HINT_SUFFIX(malloc, nonnull(1));

MuonSwitchCase *muon_switch_case(
    MuonEngine *engine, MuonName *name, MuonExpr *expr)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonSwitchExpr *muon_switch_expr(
    MuonEngine *engine, size_t argc, MuonSwitchCase *const argv[argc])
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonSequenceExpr *muon_sequence_expr(
    MuonEngine *engine, size_t argc, MuonStmt *const argv[argc])
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonVectorExpr *muon_vector_expr(
    MuonEngine *engine, size_t argc, MuonExpr *const argv[/* argc */])
  MUON_HINT_SUFFIX(malloc, nonnull(1));

MuonBooleanSign *muon_boolean_sign(MuonEngine *engine)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonIntegerSign *muon_integer_sign(MuonEngine *engine)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonLambdaSign *muon_lambda_sign(
    MuonEngine *engine, MuonSign *argument, MuonSign *output)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonNameSign *muon_name_sign(MuonEngine *engine, MuonName *name)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonRecordSign *muon_record_sign(
    MuonEngine *engine, size_t argc, const MuonSignMember argv[/* argc */])
  MUON_HINT_SUFFIX(malloc, nonnull(1));

MuonVectorSign *muon_vector_sign(MuonEngine *engine, MuonSign *matter)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonCoercionStmt *muon_coercion_stmt(
    MuonEngine *engine, MuonSign *source, MuonSign *target, MuonExpr *expr)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonDatatypeOption *muon_datatype_option(MuonEngine *engine, MuonName *name)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonDatatypeStmt *muon_datatype_stmt(
    MuonEngine *engine,
    MuonName *name,
    size_t argc,
    MuonDatatypeOption *argv[/* argc */])
  MUON_HINT_SUFFIX(malloc, nonnull(1, 2));

MuonDefineStmt *muon_define_stmt(
    MuonEngine *engine, MuonName *name, MuonExpr *expr)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonViewMember *muon_view_member(
    MuonEngine *engine, MuonName *name, MuonView *view)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonRecordView *muon_record_view(
    MuonEngine *engine, size_t argc, MuonViewMember *argv[/* argc */])
  MUON_HINT_SUFFIX(malloc, nonnull(1));

MuonVariableView *muon_variable_view(MuonEngine *engine, MuonName *name)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonScript *muon_script(
    MuonEngine *engine, size_t argc, MuonStmt *argv[/* argc */])
  MUON_HINT_SUFFIX(malloc, nonnull);

/// Emit debugging information on the abstract @a node to the debug stream
void muon_node_debug(MuonNode *node)
  MUON_HINT_SUFFIX(nonnull);

#endif /* MUON_ENGINE_NODE_H */
