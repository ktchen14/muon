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

/// An enumeration over each kind of node, e.g. @c MUON_ACCESS_EXPR_NODE
typedef enum {
#define MUON_EMIT(T, l, UPPER) MUON_##UPPER##_NODE,
  MUON_EACH_NODE_STEM(MUON_EMIT)

  /// Equivalent to the maximum enumerator in MuonNodeEnumerator
  MUON_MINORANT_NODE = MUON_INDIRECT(MUON_TAKE, MUON_EACH_NODE_STEM(MUON_EMIT)),
#undef MUON_EMIT

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

#define MUON_EMIT(T, l, UPPER) MUON_##UPPER,
  /// Equivalent to the minimum enumerator in MuonExprEnumerator
  MUON_MINORANT_EXPR = MUON_INDIRECT(MUON_TAKE, MUON_EACH_EXPR_STEM(MUON_EMIT)),
#undef MUON_EMIT
} MuonExprTag;

/// An enumeration over each kind of sign, e.g. @c MUON_BOOLEAN_SIGN
typedef enum {
#define MUON_EMIT(T, l, UPPER) MUON_##UPPER = MUON_##UPPER##_NODE,
  MUON_EACH_SIGN_STEM(MUON_EMIT)
#undef MUON_EMIT

#define MUON_EMIT(T, l, UPPER) MUON_##UPPER,
  /// Equivalent to the minimum enumerator in MuonSignEnumerator
  MUON_MINORANT_SIGN = MUON_INDIRECT(MUON_TAKE, MUON_EACH_SIGN_STEM(MUON_EMIT)),
#undef MUON_EMIT
} MuonSignTag;

/// An enumeration over each kind of stmt, e.g. @c MUON_COERCION_STMT
typedef enum {
#define MUON_EMIT(T, l, UPPER) MUON_##UPPER = MUON_##UPPER##_NODE,
  MUON_EACH_STMT_STEM(MUON_EMIT)
#undef MUON_EMIT

#define MUON_EMIT(T, l, UPPER) MUON_##UPPER,
  /// Equivalent to the minimum enumerator in MuonSignEnumerator
  MUON_MINORANT_STMT = MUON_INDIRECT(MUON_TAKE, MUON_EACH_STMT_STEM(MUON_EMIT)),
#undef MUON_EMIT
} MuonStmtTag;

/// An enumeration over each kind of view, e.g. @c MUON_RECORD_VIEW
typedef enum {
#define MUON_EMIT(T, l, UPPER) MUON_##UPPER = MUON_##UPPER##_NODE,
  MUON_EACH_VIEW_STEM(MUON_EMIT)
#undef MUON_EMIT

#define MUON_EMIT(T, l, UPPER) MUON_##UPPER,
  /// Equivalent to the minimum enumerator in MuonSignEnumerator
  MUON_MINORANT_VIEW = MUON_INDIRECT(MUON_TAKE, MUON_EACH_VIEW_STEM(MUON_EMIT)),
#undef MUON_EMIT
} MuonViewTag;

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

/// @internal Used to emit each branch in MUON_STATOR_ENUMERATOR(), etc.
#define MUON_ENUMERATOR_EMIT(Title, l, UPPER, SUFFIX) \
  , Muon##Title *: MUON_##UPPER##SUFFIX \
  , struct Muon##Title *: MUON_##UPPER##SUFFIX

#endif /* MUON_ENGINE_COMMON_H */
