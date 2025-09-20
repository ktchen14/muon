#ifndef MUON_ENGINE_NODE_H
#define MUON_ENGINE_NODE_H

#include "common.h"
#include "name.h"

#include <stddef.h>
#include <stdint.h>

/// Expands to emit(lower, upper, title, ...) for each kind of expr
#define MUON_EACH_EXPR_STEM(emit, ...) \
  emit(access, ACCESS, Access, ##__VA_ARGS__) \
  emit(boolean, BOOLEAN, Boolean, ##__VA_ARGS__) \
  emit(cast, CAST, Cast, ##__VA_ARGS__) \
  emit(integer, INTEGER, Integer, ##__VA_ARGS__) \
  emit(invoke, INVOKE, Invoke, ##__VA_ARGS__) \
  emit(lambda, LAMBDA, Lambda, ##__VA_ARGS__) \
  emit(name, NAME, Name, ##__VA_ARGS__) \
  emit(native, NATIVE, Native, ##__VA_ARGS__) \
  emit(record, RECORD, Record, ##__VA_ARGS__) \
  emit(sequence, SEQUENCE, Sequence, ##__VA_ARGS__) \
  emit(switch, SWITCH, Switch, ##__VA_ARGS__) \
  emit(vector, VECTOR, Vector, ##__VA_ARGS__)

/// Expands to emit(lower, upper, title, ...) for each kind of sign
#define MUON_EACH_SIGN_STEM(emit, ...) \
  emit(boolean, BOOLEAN, Boolean, ##__VA_ARGS__) \
  emit(integer, INTEGER, Integer, ##__VA_ARGS__) \
  emit(lambda, LAMBDA, Lambda, ##__VA_ARGS__) \
  emit(name, NAME, Name, ##__VA_ARGS__) \
  emit(record, RECORD, Record, ##__VA_ARGS__) \
  emit(vector, VECTOR, Vector, ##__VA_ARGS__)

/// Expands to emit(lower, upper, title, ...) for each kind of stmt
#define MUON_EACH_STMT_STEM(emit, ...) \
  emit(coercion, COERCION, Coercion, ##__VA_ARGS__) \
  emit(datatype, DATATYPE, Datatype, ##__VA_ARGS__) \
  emit(define, DEFINE, Define, ##__VA_ARGS__) \

/// Expands to emit(lower, upper, title, ...) for each kind of view
#define MUON_EACH_VIEW_STEM(emit, ...) \
  emit(record, RECORD, Record, ##__VA_ARGS__) \
  emit(variable, VARIABLE, Variable, ##__VA_ARGS__)

/// @internal Used as @c emit in MU_EACH_NODE_KIND
#define MU_EACH_NODE_EMIT(l, u, t, emit, lsuffix, usuffix, tsuffix, ...) \
  emit(l##lsuffix, u##usuffix, t##tsuffix, ##__VA_ARGS__)

/// Expands to emit(lower, upper, title, ...) for each kind of node
#define MU_EACH_NODE_KIND(emit, ...) \
  MUON_EACH_EXPR_STEM(MU_EACH_NODE_EMIT, emit, \
    _expr, _EXPR, Expr, ##__VA_ARGS__) \
  MUON_EACH_SIGN_STEM(MU_EACH_NODE_EMIT, emit, \
    _sign, _SIGN, Sign, ##__VA_ARGS__) \
  MUON_EACH_STMT_STEM(MU_EACH_NODE_EMIT, emit, \
    _stmt, _STMT, Stmt, ##__VA_ARGS__) \
  MUON_EACH_VIEW_STEM(MU_EACH_NODE_EMIT, emit, \
    _view, _VIEW, View, ##__VA_ARGS__) \
  emit(expr_member, EXPR_MEMBER, ExprMember, ##__VA_ARGS__) \
  emit(switch_case, SWITCH_CASE, SwitchCase, ##__VA_ARGS__) \
  emit(datatype_option, DATATYPE_OPTION, DatatypeOption, ##__VA_ARGS__) \
  emit(view_member, VIEW_MEMBER, ViewMember, ##__VA_ARGS__)

/// An enumeration over each kind of node, e.g. @c MUON_ACCESS_EXPR_NODE
typedef enum {
#define MUON_EMIT(l, upper, t) MUON_##upper##_NODE,
  MU_EACH_NODE_KIND(MUON_EMIT)
#undef MUON_EMIT

  MUON_EXPR_MEMBER = MUON_EXPR_MEMBER_NODE,
  MUON_SWITCH_CASE = MUON_SWITCH_CASE_NODE,
  MUON_DATATYPE_OPTION = MUON_DATATYPE_OPTION_NODE,
  MUON_VIEW_MEMBER = MUON_VIEW_MEMBER_NODE,

#define MUON_EMIT(l, upper, t) MUON_##upper##_NODE,
  /// Equivalent to the minimum enumerator in MuonNodeKind
  MUON_NODE_MINORANT = MUON_INDIRECT(MUON_TAKE, MU_EACH_NODE_KIND(MUON_EMIT)),
#undef MUON_EMIT

#define MUON_EMIT(...) + 1
  /// Equivalent to the maximum enumerator in MuonNodeKind
  MUON_NODE_MAJORANT = MUON_NODE_MINORANT MU_EACH_NODE_KIND(MUON_EMIT) - 1,
#undef MUON_EMIT
} MuonNodeKind;

/// An enumeration over each kind of expr, e.g. @c MUON_ACCESS_EXPR
typedef enum {
#define MUON_EMIT(l, upper, t) MUON_##upper##_EXPR = MUON_##upper##_EXPR_NODE,
  MUON_EACH_EXPR_STEM(MUON_EMIT)
#undef MUON_EMIT

#define MUON_EMIT(l, upper, t) MUON_##upper##_EXPR,
  /// Equivalent to the minimum enumerator in MuonExprKind
  MUON_EXPR_MINORANT = MUON_INDIRECT(MUON_TAKE, MUON_EACH_EXPR_STEM(MUON_EMIT)),
#undef MUON_EMIT

#define MUON_EMIT(...) + 1
  /// Equivalent to the maximum enumerator in MuonExprKind
  MUON_EXPR_MAJORANT = MUON_EXPR_MINORANT MUON_EACH_EXPR_STEM(MUON_EMIT) - 1,
#undef MUON_EMIT
} MuonExprKind;

/// An enumeration over each kind of sign, e.g. @c MUON_BOOLEAN_SIGN
typedef enum {
#define MUON_EMIT(l, upper, t) MUON_##upper##_SIGN = MUON_##upper##_SIGN_NODE,
  MUON_EACH_SIGN_STEM(MUON_EMIT)
#undef MUON_EMIT

#define MUON_EMIT(l, upper, t) MUON_##upper##_SIGN,
  /// Equivalent to the minimum enumerator in MuonSignKind
  MUON_SIGN_MINORANT = MUON_INDIRECT(MUON_TAKE, MUON_EACH_SIGN_STEM(MUON_EMIT)),
#undef MUON_EMIT

#define MUON_EMIT(...) + 1
  /// Equivalent to the maximum enumerator in MuonSignKind
  MUON_SIGN_MAJORANT = MUON_SIGN_MINORANT MUON_EACH_SIGN_STEM(MUON_EMIT) - 1,
#undef MUON_EMIT
} MuonSignKind;

/// An enumeration over each kind of stmt, e.g. @c MUON_DEFINE_STMT
typedef enum {
#define MUON_EMIT(l, upper, t) MUON_##upper##_STMT = MUON_##upper##_STMT_NODE,
  MUON_EACH_STMT_STEM(MUON_EMIT)
#undef MUON_EMIT
} MuonStmtKind;

/// An enumeration over each kind of view, e.g. @c MUON_VARIABLE_VIEW
typedef enum {
#define MUON_EMIT(l, upper, t) MUON_##upper##_VIEW = MUON_##upper##_VIEW_NODE,
  MUON_EACH_VIEW_STEM(MUON_EMIT)
#undef MUON_EMIT
} MuonViewKind;

/**
 * @brief An abstract node
 *
 * Note that a MuonNode is a constant object; the mutable equivalent is a
 * struct MuonNode.
 */
typedef const struct MuonNode {
  MuonNodeKind kind;
  const MuonEngine *engine;
  size_t id;
} MuonNode;

/// The header that each concrete node must have
#define MUON_NODE_HEADER struct MuonNode as_node

/**
 * @brief An abstract expr
 *
 * Note that a MuonExpr is a constant object; the mutable equivalent is a
 * struct MuonExpr.
 */
typedef const struct MuonExpr {
  union { MUON_NODE_HEADER; MuonExprKind kind; };
} MuonExpr;

/**
 * @brief An abstract sign
 *
 * Note that a MuonSign is a constant object; the mutable equivalent is a
 * struct MuonSign.
 */
typedef const struct MuonSign {
  union { MUON_NODE_HEADER; MuonSignKind kind; };
} MuonSign;

/**
 * @brief An abstract stmt
 *
 * Note that a MuonStmt is a constant object; the mutable equivalent is a
 * struct MuonStmt.
 */
typedef const struct MuonStmt {
  union { MUON_NODE_HEADER; MuonStmtKind kind; };
} MuonStmt;

/**
 * @brief An abstract view
 *
 * Note that a MuonView is a constant object; the mutable equivalent is a
 * struct MuonView.
 */
typedef const struct MuonView {
  union { MUON_NODE_HEADER; MuonViewKind kind; };
} MuonView;

/// The header that each concrete expr must have
#define MUON_EXPR_HEADER union { \
  struct MuonExpr as_expr; struct MuonNode as_node; \
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
  MuonExprMember *argv[] __attribute__((counted_by(argc)));
} MuonRecordExpr;

typedef const struct MuonSequenceExpr {
  MUON_EXPR_HEADER;
  size_t argc;
  MuonStmt *argv[] __attribute__((counted_by(argc)));
} MuonSequenceExpr;

typedef const struct MuonSwitchCase {
  MUON_NODE_HEADER;
  MuonName *name;
  MuonExpr *expr;
} MuonSwitchCase;

typedef const struct MuonSwitchExpr {
  MUON_EXPR_HEADER;
  size_t argc;
  MuonSwitchCase *argv[] __attribute__((counted_by(argc)));
} MuonSwitchExpr;

typedef const struct MuonVectorExpr {
  MUON_EXPR_HEADER;
  size_t argc;
  MuonExpr *argv[] __attribute__((counted_by(argc)));
} MuonVectorExpr;

MuonAccessExpr *muon_access_expr(MuonEngine *engine, MuonName *name)
  __attribute__((malloc, nonnull));

MuonBooleanExpr *muon_boolean_expr(MuonEngine *engine, _Bool data)
  __attribute__((malloc, nonnull));

MuonCastExpr *muon_cast_expr(
    MuonEngine *engine, MuonSign *sign, MuonExpr *matter)
  __attribute__((malloc, nonnull));

MuonIntegerExpr *muon_integer_expr(MuonEngine *engine, uint64_t data)
  __attribute__((malloc, nonnull));

MuonInvokeExpr *muon_invoke_expr(
    MuonEngine *engine, MuonExpr *operator, MuonExpr *argument)
  __attribute__((malloc, nonnull));

MuonLambdaExpr *muon_lambda_expr(
    MuonEngine *engine, MuonView *argument, MuonExpr *matter)
  __attribute__((malloc, nonnull));

MuonNameExpr *muon_name_expr(MuonEngine *engine, MuonName *name)
  __attribute__((malloc, nonnull));

MuonNativeExpr *muon_native_expr(MuonEngine *engine, MuonName *name)
  __attribute__((malloc, nonnull));

MuonExprMember *muon_expr_member(
    MuonEngine *engine, MuonName *name, MuonExpr *expr)
  __attribute__((malloc, nonnull));

MuonRecordExpr *muon_record_expr(
    MuonEngine *engine, size_t argc, MuonExprMember *argv[/* argc */])
  __attribute__((malloc, nonnull(1)));

MuonSwitchCase *muon_switch_case(
    MuonEngine *engine, MuonName *name, MuonExpr *expr)
  __attribute__((malloc, nonnull));

MuonSwitchExpr *muon_switch_expr(
    MuonEngine *engine, size_t argc, MuonSwitchCase *const argv[argc])
  __attribute__((malloc, nonnull));

MuonSequenceExpr *muon_sequence_expr(
    MuonEngine *engine, size_t argc, MuonStmt *const argv[argc])
  __attribute__((malloc, nonnull));

MuonVectorExpr *muon_vector_expr(
    MuonEngine *engine, size_t argc, MuonExpr *const argv[/* argc */])
  __attribute__((malloc, nonnull(1)));

/// The header that each concrete sign must have
#define MUON_SIGN_HEADER union { \
  struct MuonSign as_sign; struct MuonNode as_node; \
}

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
  MuonSignMember argv[] __attribute__((counted_by(argc)));
} MuonRecordSign;

typedef const struct MuonVectorSign {
  MUON_SIGN_HEADER;
  MuonSign *matter;
} MuonVectorSign;

MuonBooleanSign *muon_boolean_sign(MuonEngine *engine)
  __attribute__((malloc, nonnull));

MuonIntegerSign *muon_integer_sign(MuonEngine *engine)
  __attribute__((malloc, nonnull));

MuonLambdaSign *muon_lambda_sign(
    MuonEngine *engine, MuonSign *argument, MuonSign *output)
  __attribute__((malloc, nonnull));

MuonNameSign *muon_name_sign(MuonEngine *engine, MuonName *name)
  __attribute__((malloc, nonnull));

MuonRecordSign *muon_record_sign(
    MuonEngine *engine, size_t argc, const MuonSignMember argv[/* argc */])
  __attribute__((malloc, nonnull(1)));

MuonVectorSign *muon_vector_sign(MuonEngine *engine, MuonSign *matter)
  __attribute__((malloc, nonnull));

/// The header that each concrete stmt must have
#define MUON_STMT_HEADER union { \
  struct MuonStmt as_stmt; struct MuonNode as_node; \
}

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
  MuonName *argv[] __attribute__((counted_by(argc)));
} MuonTypeNode;

typedef const struct MuonDatatypeOption {
  MUON_NODE_HEADER;
  MuonName *name;
} MuonDatatypeOption;

typedef const struct MuonDatatypeStmt {
  MUON_STMT_HEADER;
  MuonName *name;
  size_t argc;
  MuonDatatypeOption *argv[] __attribute__((counted_by(argc)));
} MuonDatatypeStmt;

typedef const struct MuonDefineStmt {
  MUON_STMT_HEADER;
  MuonName *name;
  MuonExpr *expr;
} MuonDefineStmt;

MuonCoercionStmt *muon_coercion_stmt(
    MuonEngine *engine, MuonSign *source, MuonSign *target, MuonExpr *expr)
  __attribute__((malloc, nonnull));

MuonDatatypeOption *muon_datatype_option(MuonEngine *engine, MuonName *name)
  __attribute__((malloc, nonnull));

MuonDatatypeStmt *muon_datatype_stmt(
    MuonEngine *engine,
    MuonName *name,
    size_t argc,
    MuonDatatypeOption *argv[/* argc */])
  __attribute__((malloc, nonnull(1, 2)));

MuonDefineStmt *muon_define_stmt(
    MuonEngine *engine, MuonName *name, MuonExpr *expr)
  __attribute__((malloc, nonnull));

/// The header that each concrete view must have
#define MUON_VIEW_HEADER union { \
  struct MuonView as_view; struct MuonNode as_node; \
}

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
  MuonViewMember *argv[] __attribute__((counted_by(argc)));
} MuonRecordView;

typedef const struct MuonVariableView {
  MUON_VIEW_HEADER;
  MuonName *name;
} MuonVariableView;

MuonViewMember *muon_view_member(
    MuonEngine *engine, MuonName *name, MuonView *view)
  __attribute__((malloc, nonnull));

MuonRecordView *muon_record_view(
    MuonEngine *engine, size_t argc, MuonViewMember *argv[/* argc */])
  __attribute__((malloc, nonnull(1)));

MuonVariableView *muon_variable_view(MuonEngine *engine, MuonName *name)
  __attribute__((malloc, nonnull));

/// Emit debugging information on the abstract @a node to the debug stream
void muon_node_debug(MuonNode *node) __attribute__((nonnull));

/// @internal Used to emit each branch in MU_NODE_ENUMERATOR() et al
#define MUON_ENUMERATOR_EMIT(l, upper, title, usuffix, tsuffix) \
  , Muon##title##tsuffix *: MUON_##upper##usuffix

/**
 * @brief Return the minimum enumerator indicative of the concrete @a node
 *
 * @a node must be a pointer to a concrete node.
 *
 * This is equivalent to MU_NODE_ENUMERATOR() if @a node is the type of a
 * concrete node. However, if @a node is <tt>MuonExpr *</tt>, then this returns
 * MU_EXPR_MINORANT.
 */
#define MUON_NODE_ENUMERATOR_MINIMUM(node) \
  _Generic((node) MU_EACH_NODE_KIND(MUON_ENUMERATOR_EMIT, _NODE,), \
    MuonExpr *: MUON_EXPR_MINORANT, \
    MuonSign *: MUON_SIGN_MINORANT)

/**
 * @brief Return the maximum enumerator indicative of the concrete @a node
 *
 * @a node must be a pointer to a concrete node.
 *
 * This is equivalent to MU_NODE_ENUMERATOR() if @a node is the type of a
 * concrete node. However, if @a node is <tt>MuonExpr *</tt>, then this returns
 * MU_EXPR_MAJORANT.
 */
#define MUON_NODE_ENUMERATOR_MAXIMUM(node) \
  _Generic((node) MU_EACH_NODE_KIND(MUON_ENUMERATOR_EMIT, _NODE,), \
    MuonExpr *: MUON_EXPR_MAJORANT, \
    MuonSign *: MUON_SIGN_MINORANT)

/// Return the enumerator indicative of the concrete @a node
#define MU_NODE_ENUMERATOR(node) \
  _Generic((node) MU_EACH_NODE_KIND(MUON_ENUMERATOR_EMIT, _NODE,))

/// Return the enumerator indicative of the type of the concrete @a expr
#define MUON_EXPR_ENUMERATOR(expr) _Generic((expr) \
  MUON_EACH_EXPR_STEM(MUON_ENUMERATOR_EMIT, _EXPR, Expr) \
)

/// Return the enumerator indicative of the type of the concrete @a sign
#define MUON_SIGN_ENUMERATOR(sign) _Generic((sign) \
  MUON_EACH_SIGN_STEM(MUON_ENUMERATOR_EMIT, _SIGN, Sign) \
)

/// @internal Used to emit each branch in mu_stmt_cast()
#define MU_STMT_CAST_EMIT(l, upper, title, ...) \
  , Muon##title##Stmt *: _kind == MUON_##upper##_STMT##__VA_ARGS__

/// @internal Used to emit each branch in mu_view_cast()
#define MU_VIEW_CAST_EMIT(l, upper, title, ...) \
  , Muon##title##View *: _kind == MUON_##upper##_VIEW##__VA_ARGS__

/// @internal Used to decide the cast result in muon_node_cast()
__attribute__((nonnull))
static inline MuonNode *muon_node_cast(
    MuonNode *node, MuonNodeKind minimum, MuonNodeKind maximum) {
  MuonNodeKind kind = node->kind;
  return kind >= minimum && kind <= maximum ? node : NULL;
}

/// @internal Used to decide the cast result in muon_expr_cast()
__attribute__((nonnull))
static inline MuonExpr *muon_expr_cast(MuonExpr *expr, MuonExprKind kind) {
  return expr->kind == kind ? expr : NULL;
}

/// @internal Used to decide the cast result in muon_sign_cast()
__attribute__((nonnull))
static inline MuonSign *muon_sign_cast(MuonSign *sign, MuonSignKind kind) {
  return sign->kind == kind ? sign : NULL;
}

/**
 * @brief Downcast the @a abstract node to the <tt>typeof(concrete)</tt>
 *
 * @a abstract should have type <tt>MuonNode *</tt>. @a concrete should be, or
 * have, the type of a pointer to a const qualified concrete node. Then if
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
  (__typeof__((concrete))) (muon_node_cast)((node), \
    MUON_NODE_ENUMERATOR_MINIMUM((concrete)), \
    MUON_NODE_ENUMERATOR_MAXIMUM((concrete))) \
)

/**
 * @brief Downcast the abstract @a expr to the <tt>typeof(concrete)</tt>
 *
 * @a abstract should have type <tt>MuonExpr *</tt>. @a concrete should be, or
 * have, the type of a pointer to a const qualified concrete expr. Then if
 * @a abstract is an instance of that type, it will be cast to that type and
 * returned. Otherwise, this will return @c NULL.
 *
 * @par Example:
 * @code{.c}
 *   MuonExpr *abstract_expr = ...;
 *
 *   MuonAccessExpr *expr;
 *   if ((expr = mu_expr_cast(abstract_expr, expr)) == NULL)
 *     return ...;
 * @endcode
 *
 * The behavior is undefined if:
 * - @a abstract is @c NULL
 * - @a abstract doesn't have type <tt>MuonExpr *</tt>
 * - @a concrete isn't, or doesn't have, the type of a const qualified pointer
 *   to a concrete expr
 */
#define muon_expr_cast(expr, concrete) ( \
  (__typeof__((concrete))) (muon_expr_cast)((expr), \
    MUON_EXPR_ENUMERATOR((concrete))) \
)

/**
 * @brief Downcast the abstract @a sign to the <tt>typeof(concrete)</tt>
 *
 * @a abstract should have type <tt>MuonSign *</tt>. @a concrete should be, or
 * have, the type of a pointer to a const qualified concrete sign. Then if
 * @a abstract is an instance of that type, it will be cast to that type and
 * returned. Otherwise, this will return @c NULL.
 *
 * @par Example:
 * @code{.c}
 *   MuonSign *abstract_sign = ...;
 *
 *   MuonVectorSign *sign;
 *   if ((sign = mu_sign_cast(abstract_sign, sign)) == NULL)
 *     return ...;
 * @endcode
 *
 * The behavior is undefined if:
 * - @a abstract is @c NULL
 * - @a abstract doesn't have type <tt>MuonSign *</tt>
 * - @a concrete isn't, or doesn't have, the type of a const qualified pointer
 *   to a concrete sign
 */
#define mu_sign_cast(sign, concrete) ( \
  (__typeof__((concrete))) (muon_sign_cast)((sign), \
    MUON_SIGN_ENUMERATOR((concrete))) \
)

/**
 * @brief Downcast the @a abstract stmt to the <tt>typeof(concrete)</tt>
 *
 * @a abstract should have type <tt>MuonStmt *</tt>. @a concrete should be, or
 * have, the type of a pointer to a const qualified concrete stmt. Then if
 * @a abstract is an instance of that type, it will be cast to that type and
 * returned. Otherwise, this will return @c NULL.
 *
 * @par Example:
 * @code{.c}
 *   MuonStmt *abstract_stmt = ...;
 *
 *   MuonVectorStmt *stmt;
 *   if ((stmt = mu_stmt_cast(abstract_stmt, stmt)) == NULL)
 *     return ...;
 * @endcode
 *
 * The behavior is undefined if:
 * - @a abstract is @c NULL
 * - @a abstract doesn't have type <tt>MuonStmt *</tt>
 * - @a concrete isn't, or doesn't have, the type of a const qualified pointer
 *   to a concrete stmt
 */
#define mu_stmt_cast(abstract, concrete) __extension__ ({ \
    MuonStmt *_abstract = (abstract); \
    typeof(concrete) _concrete; \
    MuonStmtKind _kind = _abstract->kind; \
    int _castable = _Generic(_concrete MUON_EACH_STMT_STEM(MU_STMT_CAST_EMIT)); \
    _castable ? (typeof(_concrete)) _abstract : NULL; \
  })

/**
 * @brief Downcast the @a abstract view to the <tt>typeof(concrete)</tt>
 *
 * @a abstract should have type <tt>MuonView *</tt>. @a concrete should
 * be, or have, the type of a pointer to a const qualified concrete view. Then
 * if @a abstract is an instance of that type, it will be cast to that type and
 * returned. Otherwise, this will return @c NULL.
 *
 * @par Example:
 * @code{.c}
 *   MuonView *abstract_view = ...;
 *
 *   MuonVectorView *view;
 *   if ((view = mu_view_cast(abstract_view, view)) == NULL)
 *     return ...;
 * @endcode
 *
 * The behavior is undefined if:
 * - @a abstract is @c NULL
 * - @a abstract doesn't have type <tt>MuonView *</tt>
 * - @a concrete isn't, or doesn't have, the type of a const qualified pointer
 *   to a concrete view
 */
#define mu_view_cast(abstract, concrete) __extension__ ({ \
    MuonView *_abstract = (abstract); \
    typeof(concrete) _concrete; \
    MuonViewKind _kind = _abstract->kind; \
    int _castable = _Generic(_concrete MUON_EACH_VIEW_STEM(MU_VIEW_CAST_EMIT)); \
    _castable ? (typeof(_concrete)) _abstract : NULL; \
  })

#endif /* MUON_ENGINE_NODE_H */
