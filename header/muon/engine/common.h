#ifndef MUON_ENGINE_COMMON_H
#define MUON_ENGINE_COMMON_H

#include "../common.h" // IWYU pragma: export

#include <limits.h>
#include <stddef.h>

typedef struct {
  void *remote;
  _Alignas(max_align_t) char data[sizeof(void *[256])];
} MuonEngine;

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

/// Expands to emit(Title, lower, UPPER, ...) for each kind of stator
#define MUON_EACH_STATOR_STEM(emit, ...) \
  emit(Name, name, NAME __VA_OPT__(,) __VA_ARGS__) \
  MUON_EACH_NODE_STEM(emit __VA_OPT__(,) __VA_ARGS__)

/// An enumeration over each kind of stator, e.g. @c MUON_NAME_STATOR
typedef enum {
#define MUON_EMIT(T, l, UPPER) MUON_##UPPER##_STATOR,
  MUON_EACH_STATOR_STEM(MUON_EMIT)
#undef MUON_EMIT

/// @internal Expands to <tt>emit(...)</tt>
#define MUON_INDIRECT(emit, ...) emit(__VA_ARGS__)

/// @internal Expands to @a argument
#define MUON_TAKE(argument, ...) argument

#define MUON_EMIT(T, l, UPPER) MUON_##UPPER##_STATOR,
  /// Equivalent to the minimum enumerator in MuonStatorEnumerator
  MUON_MINORANT_STATOR = MUON_INDIRECT(
      MUON_TAKE, MUON_EACH_STATOR_STEM(MUON_EMIT)),

  /// Equivalent to the maximum enumerator in MuonNodeEnumerator
  MUON_MINORANT_NODE_STATOR = MUON_INDIRECT(
      MUON_TAKE, MUON_EACH_NODE_STEM(MUON_EMIT)),

  /// Equivalent to the minimum enumerator in MuonExprEnumerator
  MUON_MINORANT_EXPR_STATOR = MUON_INDIRECT(
      MUON_TAKE, MUON_EACH_EXPR_STEM(MUON_EMIT)),

  /// Equivalent to the minimum enumerator in MuonSignEnumerator
  MUON_MINORANT_SIGN_STATOR = MUON_INDIRECT(
      MUON_TAKE, MUON_EACH_SIGN_STEM(MUON_EMIT)),

  /// Equivalent to the minimum enumerator in MuonStmtEnumerator
  MUON_MINORANT_STMT_STATOR = MUON_INDIRECT(
      MUON_TAKE, MUON_EACH_STMT_STEM(MUON_EMIT)),

  /// Equivalent to the minimum enumerator in MuonStmtEnumerator
  MUON_MINORANT_VIEW_STATOR = MUON_INDIRECT(
      MUON_TAKE, MUON_EACH_VIEW_STEM(MUON_EMIT)),
#undef MUON_EMIT
#undef MUON_TAKE
#undef MUON_INDIRECT

#define MUON_EMIT(...) 1 +
  /// Equivalent to the maximum enumerator in MuonStatorEnumerator
  MUON_MAJORANT_STATOR = MUON_MINORANT_STATOR + MUON_EACH_STATOR_STEM(MUON_EMIT)
      - 1,

  /// Equivalent to the maximum enumerator in MuonNodeEnumerator
  MUON_MAJORANT_NODE_STATOR = MUON_MINORANT_NODE_STATOR
      + MUON_EACH_NODE_STEM(MUON_EMIT) - 1,

  /// Equivalent to the maximum enumerator in MuonExprEnumerator
  MUON_MAJORANT_EXPR_STATOR = MUON_MINORANT_EXPR_STATOR
      + MUON_EACH_EXPR_STEM(MUON_EMIT) - 1,

  /// Equivalent to the maximum enumerator in MuonSignEnumerator
  MUON_MAJORANT_SIGN_STATOR = MUON_MINORANT_SIGN_STATOR
      + MUON_EACH_SIGN_STEM(MUON_EMIT) - 1,

  /// Equivalent to the maximum enumerator in MuonStmtEnumerator
  MUON_MAJORANT_STMT_STATOR = MUON_MINORANT_STMT_STATOR
      + MUON_EACH_STMT_STEM(MUON_EMIT) - 1,

  /// Equivalent to the maximum enumerator in MuonStmtEnumerator
  MUON_MAJORANT_VIEW_STATOR = MUON_MINORANT_VIEW_STATOR
      + MUON_EACH_VIEW_STEM(MUON_EMIT) - 1,
#undef MUON_EMIT
} MuonStatorEnumerator;

/// An enumeration over each kind of node, e.g. @c MUON_ACCESS_EXPR_NODE
typedef enum {
#define MUON_EMIT(T, l, UPPER) MUON_##UPPER##_NODE = MUON_##UPPER##_STATOR,
  MUON_EACH_NODE_STEM(MUON_EMIT)
#undef MUON_EMIT

  MUON_EXPR_MEMBER = MUON_EXPR_MEMBER_NODE, //-
  MUON_SWITCH_CASE = MUON_SWITCH_CASE_NODE,
  MUON_DATATYPE_OPTION = MUON_DATATYPE_OPTION_NODE,
  MUON_VIEW_MEMBER = MUON_VIEW_MEMBER_NODE,
  MUON_SCRIPT = MUON_SCRIPT_NODE,

  /// Equivalent to the minimum enumerator in MuonNodeEnumerator
  MUON_MINORANT_NODE = MUON_MINORANT_NODE_STATOR,

  /// Equivalent to the minimum enumerator in MuonExprEnumerator
  MUON_MINORANT_EXPR_NODE = MUON_MINORANT_EXPR_STATOR,

  /// Equivalent to the minimum enumerator in MuonSignEnumerator
  MUON_MINORANT_SIGN_NODE = MUON_MINORANT_SIGN_STATOR,

  /// Equivalent to the minimum enumerator in MuonStmtEnumerator
  MUON_MINORANT_STMT_NODE = MUON_MINORANT_STMT_STATOR,

  /// Equivalent to the minimum enumerator in MuonViewEnumerator
  MUON_MINORANT_VIEW_NODE = MUON_MINORANT_VIEW_STATOR,

  /// Equivalent to the maximum enumerator in MuonNodeEnumerator
  MUON_MAJORANT_NODE = MUON_MAJORANT_NODE_STATOR,

  /// Equivalent to the maximum enumerator in MuonExprEnumerator
  MUON_MAJORANT_EXPR_NODE = MUON_MAJORANT_EXPR_STATOR,

  /// Equivalent to the maximum enumerator in MuonSignEnumerator
  MUON_MAJORANT_SIGN_NODE = MUON_MAJORANT_SIGN_STATOR,

  /// Equivalent to the maximum enumerator in MuonStmtEnumerator
  MUON_MAJORANT_STMT_NODE = MUON_MAJORANT_STMT_STATOR,

  /// Equivalent to the maximum enumerator in MuonViewEnumerator
  MUON_MAJORANT_VIEW_NODE = MUON_MAJORANT_VIEW_STATOR,
} MuonNodeEnumerator;

/// An enumeration over each kind of expr, e.g. @c MUON_ACCESS_EXPR
typedef enum {
#define MUON_EMIT(T, l, UPPER) MUON_##UPPER = MUON_##UPPER##_NODE,
  MUON_EACH_EXPR_STEM(MUON_EMIT)
#undef MUON_EMIT

  /// Equivalent to the minimum enumerator in MuonExprEnumerator
  MUON_MINORANT_EXPR = MUON_MINORANT_EXPR_NODE,

  /// Equivalent to the maximum enumerator in MuonExprEnumerator
  MUON_MAJORANT_EXPR = MUON_MAJORANT_EXPR_NODE,
} MuonExprEnumerator;

/// An enumeration over each kind of sign, e.g. @c MUON_BOOLEAN_SIGN
typedef enum {
#define MUON_EMIT(T, l, UPPER) MUON_##UPPER = MUON_##UPPER##_NODE,
  MUON_EACH_SIGN_STEM(MUON_EMIT)
#undef MUON_EMIT

  /// Equivalent to the minimum enumerator in MuonSignEnumerator
  MUON_MINORANT_SIGN = MUON_MINORANT_SIGN_NODE,

  /// Equivalent to the maximum enumerator in MuonSignEnumerator
  MUON_MAJORANT_SIGN = MUON_MAJORANT_SIGN_NODE,
} MuonSignEnumerator;

/// An enumeration over each kind of stmt, e.g. @c MUON_COERCION_STMT
typedef enum {
#define MUON_EMIT(T, l, UPPER) MUON_##UPPER = MUON_##UPPER##_NODE,
  MUON_EACH_STMT_STEM(MUON_EMIT)
#undef MUON_EMIT

  /// Equivalent to the minimum enumerator in MuonStmtEnumerator
  MUON_MINORANT_STMT = MUON_MINORANT_STMT_NODE,

  /// Equivalent to the maximum enumerator in MuonStmtEnumerator
  MUON_MAJORANT_STMT = MUON_MAJORANT_STMT_NODE,
} MuonStmtEnumerator;

/// An enumeration over each kind of view, e.g. @c MUON_RECORD_VIEW
typedef enum {
#define MUON_EMIT(T, l, UPPER) MUON_##UPPER = MUON_##UPPER##_NODE,
  MUON_EACH_VIEW_STEM(MUON_EMIT)
#undef MUON_EMIT

  /// Equivalent to the minimum enumerator in MuonViewEnumerator
  MUON_MINORANT_VIEW = MUON_MINORANT_VIEW_NODE,

  /// Equivalent to the maximum enumerator in MuonViewEnumerator
  MUON_MAJORANT_VIEW = MUON_MAJORANT_VIEW_NODE,
} MuonViewEnumerator;

enum {
  /// Number of distinct kinds of stators
  MUON_STATOR_NUMBER = MUON_MAJORANT_STATOR - MUON_MINORANT_STATOR + 1,

  /// Number of distinct kinds of nodes
  MUON_NODE_NUMBER = MUON_MAJORANT_NODE - MUON_MINORANT_NODE + 1,

  /// Number of distinct kinds of exprs
  MUON_EXPR_NUMBER = MUON_MAJORANT_EXPR - MUON_MINORANT_EXPR + 1,

  /// Number of distinct kinds of signs
  MUON_SIGN_NUMBER = MUON_MAJORANT_SIGN - MUON_MINORANT_SIGN + 1,

  /// Number of distinct kinds of stmts
  MUON_STMT_NUMBER = MUON_MAJORANT_STMT - MUON_MINORANT_STMT + 1,

  /// Number of distinct kinds of views
  MUON_VIEW_NUMBER = MUON_MAJORANT_VIEW - MUON_MINORANT_VIEW + 1,
};

/// @internal Used to emit each branch in MUON_STATOR_ENUMERATOR(), etc.
#define MUON_ENUMERATOR_EMIT(Title, l, UPPER, US) \
  , Muon##Title *: MUON_##UPPER, struct Muon##Title *: MUON_##UPPER

typedef MuonNodeEnumerator MuonNodeTag;
typedef MuonExprEnumerator MuonExprTag;
typedef MuonSignEnumerator MuonSignTag;
typedef MuonStmtEnumerator MuonStmtTag;
typedef MuonViewEnumerator MuonViewTag;

#endif /* MUON_ENGINE_COMMON_H */
