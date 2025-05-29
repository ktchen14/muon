#ifndef MU_ENGINE_NODE_H
#define MU_ENGINE_NODE_H

#include "common.h"
#include "name.h"

#include <stddef.h>
#include <stdint.h>

/// Expands to emit(lower, upper, title, ...) for each kind of expr
#define MU_EACH_EXPR_KIND(emit, ...) \
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
#define MU_EACH_SIGN_KIND(emit, ...) \
  emit(boolean, BOOLEAN, Boolean, ##__VA_ARGS__) \
  emit(integer, INTEGER, Integer, ##__VA_ARGS__) \
  emit(lambda, LAMBDA, Lambda, ##__VA_ARGS__) \
  emit(name, NAME, Name, ##__VA_ARGS__) \
  emit(record, RECORD, Record, ##__VA_ARGS__) \
  emit(vector, VECTOR, Vector, ##__VA_ARGS__)

/// Expands to emit(lower, upper, title, ...) for each kind of stmt
#define MU_EACH_STMT_KIND(emit, ...) \
  emit(coercion, COERCION, Coercion, ##__VA_ARGS__) \
  emit(datatype, DATATYPE, Datatype, ##__VA_ARGS__) \
  emit(define, DEFINE, Define, ##__VA_ARGS__) \

/// Expands to emit(lower, upper, title, ...) for each kind of view
#define MU_EACH_VIEW_KIND(emit, ...) \
  emit(record, RECORD, Record, ##__VA_ARGS__) \
  emit(variable, VARIABLE, Variable, ##__VA_ARGS__)

/// @internal Used as @c emit in MU_EACH_NODE_KIND
#define MU_EACH_NODE_EMIT(l, u, t, lsuffix, usuffix, tsuffix, emit, ...) \
  emit(l##lsuffix, u##usuffix, t##tsuffix, ##__VA_ARGS__)

/// Expands to emit(lower, upper, title, ...) for each kind of node
#define MU_EACH_NODE_KIND(emit, ...) \
  MU_EACH_EXPR_KIND(MU_EACH_NODE_EMIT, _expr, _EXPR, Expr, emit, ##__VA_ARGS__) \
  MU_EACH_SIGN_KIND(MU_EACH_NODE_EMIT, _sign, _SIGN, Sign, emit, ##__VA_ARGS__) \
  MU_EACH_STMT_KIND(MU_EACH_NODE_EMIT, _stmt, _STMT, Stmt, emit, ##__VA_ARGS__) \
  MU_EACH_VIEW_KIND(MU_EACH_NODE_EMIT, _view, _VIEW, View, emit, ##__VA_ARGS__) \
  emit(expr_member, EXPR_MEMBER, ExprMember, ##__VA_ARGS__) \
  emit(switch_case, SWITCH_CASE, SwitchCase, ##__VA_ARGS__) \
  emit(datatype_option, DATATYPE_OPTION, DatatypeOption, ##__VA_ARGS__) \
  emit(view_member, VIEW_MEMBER, ViewMember, ##__VA_ARGS__)

/// An enumeration over each kind of node, e.g. @c MU_ACCESS_EXPR_NODE
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_NODE,
  MU_EACH_NODE_KIND(MU_EMIT)
#undef MU_EMIT

  MU_EXPR_MEMBER = MU_EXPR_MEMBER_NODE,
  MU_SWITCH_CASE = MU_SWITCH_CASE_NODE,
  MU_DATATYPE_OPTION = MU_DATATYPE_OPTION_NODE,
  MU_VIEW_MEMBER = MU_VIEW_MEMBER_NODE,
} MuonNodeKind;

/// An enumeration over each kind of expr, e.g. @c MU_ACCESS_EXPR
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_EXPR = MU_##upper##_EXPR_NODE,
  MU_EACH_EXPR_KIND(MU_EMIT)
#undef MU_EMIT
} MuonExprKind;

/// An enumeration over each kind of sign, e.g. @c MU_BOOLEAN_SIGN
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_SIGN = MU_##upper##_SIGN_NODE,
  MU_EACH_SIGN_KIND(MU_EMIT)
#undef MU_EMIT
} MuonSignKind;

/// An enumeration over each kind of stmt, e.g. @c MU_DEFINE_STMT
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_STMT = MU_##upper##_STMT_NODE,
  MU_EACH_STMT_KIND(MU_EMIT)
#undef MU_EMIT
} MuonStmtKind;

/// An enumeration over each kind of view, e.g. @c MU_VARIABLE_VIEW
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_VIEW = MU_##upper##_VIEW_NODE,
  MU_EACH_VIEW_KIND(MU_EMIT)
#undef MU_EMIT
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
#define MU_NODE_HEADER struct MuonNode as_node

/**
 * @brief An abstract expr
 *
 * Note that a MuonExpr is a constant object; the mutable equivalent is a
 * struct MuonExpr.
 */
typedef const struct MuonExpr {
  union { MU_NODE_HEADER; MuonExprKind kind; };
} MuonExpr;

/**
 * @brief An abstract sign
 *
 * Note that a MuonSign is a constant object; the mutable equivalent is a
 * struct MuonSign.
 */
typedef const struct MuonSign {
  union { MU_NODE_HEADER; MuonSignKind kind; };
} MuonSign;

/**
 * @brief An abstract stmt
 *
 * Note that a MuonStmt is a constant object; the mutable equivalent is a
 * struct MuonStmt.
 */
typedef const struct MuonStmt {
  union { MU_NODE_HEADER; MuonStmtKind kind; };
} MuonStmt;

/**
 * @brief An abstract view
 *
 * Note that a MuonView is a constant object; the mutable equivalent is a
 * struct MuonView.
 */
typedef const struct MuonView {
  union { MU_NODE_HEADER; MuonViewKind kind; };
} MuonView;

/// The header that each concrete expr must have
#define MU_EXPR_HEADER union { \
  struct MuonExpr as_expr; struct MuonNode as_node; \
}

typedef const struct MuonAccessExpr {
  MU_EXPR_HEADER;
  MuonName *name;
} MuonAccessExpr;

typedef const struct MuonBooleanExpr {
  MU_EXPR_HEADER;
  _Bool data;
} MuonBooleanExpr;

typedef const struct MuonCastExpr {
  MU_EXPR_HEADER;
  MuonSign *sign;
  MuonExpr *matter;
} MuonCastExpr;

typedef const struct MuonIntegerExpr {
  MU_EXPR_HEADER;
  uint64_t data;
} MuonIntegerExpr;

typedef const struct MuonInvokeExpr {
  MU_EXPR_HEADER;
  MuonExpr *operator;
  MuonExpr *argument;
} MuonInvokeExpr;

typedef const struct MuonLambdaExpr {
  MU_EXPR_HEADER;
  MuonView *argument;
  MuonExpr *matter;
} MuonLambdaExpr;

typedef const struct MuonNameExpr {
  MU_EXPR_HEADER;
  MuonName *name;
} MuonNameExpr;

typedef const struct MuonNativeExpr {
  MU_EXPR_HEADER;
  MuonName *name;
} MuonNativeExpr;

typedef const struct MuonExprMember {
  MU_NODE_HEADER;
  MuonName *name; // optional
  MuonExpr *expr;
} MuonExprMember;

typedef const struct MuonRecordExpr {
  MU_EXPR_HEADER;
  size_t argc;
  MuonExprMember *argv[/* argc */];
} MuonRecordExpr;

typedef const struct MuonSequenceExpr {
  MU_EXPR_HEADER;
  size_t argc;
  MuonStmt *argv[/* argc */];
} MuonSequenceExpr;

typedef const struct MuonSwitchCase {
  MU_NODE_HEADER;
  MuonName *name;
  MuonExpr *expr;
} MuonSwitchCase;

typedef const struct MuonSwitchExpr {
  MU_EXPR_HEADER;
  size_t argc;
  MuonSwitchCase *argv[/* argc */];
} MuonSwitchExpr;

typedef const struct MuonVectorExpr {
  MU_EXPR_HEADER;
  size_t argc;
  MuonExpr *argv[/* argc */];
} MuonVectorExpr;

MuonAccessExpr *mu_access_expr(MuonEngine *engine, MuonName *name)
  __attribute__((malloc, nonnull));

MuonBooleanExpr *mu_boolean_expr(MuonEngine *engine, _Bool data)
  __attribute__((malloc, nonnull));

MuonCastExpr *mu_cast_expr(
    MuonEngine *engine, MuonSign *sign, MuonExpr *matter)
  __attribute__((malloc, nonnull));

MuonIntegerExpr *mu_integer_expr(MuonEngine *engine, uint64_t data)
  __attribute__((malloc, nonnull));

MuonInvokeExpr *mu_invoke_expr(
    MuonEngine *engine, MuonExpr *operator, MuonExpr *argument)
  __attribute__((malloc, nonnull));

MuonLambdaExpr *mu_lambda_expr(
    MuonEngine *engine, MuonView *argument, MuonExpr *matter)
  __attribute__((malloc, nonnull));

MuonNameExpr *mu_name_expr(MuonEngine *engine, MuonName *name)
  __attribute__((malloc, nonnull));

MuonNativeExpr *mu_native_expr(MuonEngine *engine, MuonName *name)
  __attribute__((malloc, nonnull));

MuonExprMember *mu_expr_member(
    MuonEngine *engine, MuonName *name, MuonExpr *expr)
  __attribute__((malloc, nonnull));

MuonRecordExpr *mu_record_expr(
    MuonEngine *engine, size_t argc, MuonExprMember *argv[/* argc */])
  __attribute__((malloc, nonnull(1)));

MuonSwitchCase *mu_switch_case(
    MuonEngine *engine, MuonName *name, MuonExpr *expr)
  __attribute__((malloc, nonnull));

MuonSwitchExpr *mu_switch_expr(
    MuonEngine *engine, size_t argc, MuonSwitchCase *const argv[argc])
  __attribute__((malloc, nonnull));

MuonSequenceExpr *mu_sequence_expr(
    MuonEngine *engine, size_t argc, MuonStmt *const argv[argc])
  __attribute__((malloc, nonnull));

MuonVectorExpr *mu_vector_expr(
    MuonEngine *engine, size_t argc, MuonExpr *const argv[/* argc */])
  __attribute__((malloc, nonnull(1)));

/// The header that each concrete sign must have
#define MU_SIGN_HEADER union { \
  struct MuonSign as_sign; struct MuonNode as_node; \
}

typedef const struct MuonBooleanSign {
  MU_SIGN_HEADER;
} MuonBooleanSign;

typedef const struct MuonIntegerSign {
  MU_SIGN_HEADER;
} MuonIntegerSign;

typedef const struct MuonLambdaSign {
  MU_SIGN_HEADER;
  MuonSign *argument;
  MuonSign *output;
} MuonLambdaSign;

typedef const struct MuonNameSign {
  MU_SIGN_HEADER;
  MuonName *name;
} MuonNameSign;

typedef struct {
  MuonName *name; // optional
  MuonSign *sign;
} MuonSignMember;

typedef const struct MuonRecordSign {
  MU_SIGN_HEADER;
  size_t argc;
  MuonSignMember argv[/* argc */];
} MuonRecordSign;

typedef const struct MuonVectorSign {
  MU_SIGN_HEADER;
  MuonSign *matter;
} MuonVectorSign;

MuonBooleanSign *mu_boolean_sign(MuonEngine *engine)
  __attribute__((malloc, nonnull));

MuonIntegerSign *mu_integer_sign(MuonEngine *engine)
  __attribute__((malloc, nonnull));

MuonLambdaSign *mu_lambda_sign(
    MuonEngine *engine, MuonSign *argument, MuonSign *output)
  __attribute__((malloc, nonnull));

MuonNameSign *mu_name_sign(MuonEngine *engine, MuonName *name)
  __attribute__((malloc, nonnull));

MuonRecordSign *mu_record_sign(
    MuonEngine *engine, size_t argc, const MuonSignMember argv[/* argc */])
  __attribute__((malloc, nonnull(1)));

MuonVectorSign *mu_vector_sign(MuonEngine *engine, MuonSign *matter)
  __attribute__((malloc, nonnull));

/// The header that each concrete stmt must have
#define MU_STMT_HEADER union { \
  struct MuonStmt as_stmt; struct MuonNode as_node; \
}

typedef const struct MuonCoercionStmt {
  MU_STMT_HEADER;
  MuonSign *source;
  MuonSign *target;
  MuonExpr *expr;
} MuonCoercionStmt;

typedef const struct MuonTypeNode {
  MU_NODE_HEADER;
  MuonName *name;
  size_t argc;
  MuonName *argv[/* argc */];
} MuonTypeNode;

typedef const struct MuonDatatypeOption {
  MU_NODE_HEADER;
  MuonName *name;
} MuonDatatypeOption;

typedef const struct MuonDatatypeStmt {
  MU_STMT_HEADER;
  MuonName *name;
  size_t argc;
  MuonDatatypeOption *argv[/* argc */];
} MuonDatatypeStmt;

typedef const struct MuonDefineStmt {
  MU_STMT_HEADER;
  MuonName *name;
  MuonExpr *expr;
} MuonDefineStmt;

MuonCoercionStmt *mu_coercion_stmt(
    MuonEngine *engine, MuonSign *source, MuonSign *target, MuonExpr *expr)
  __attribute__((malloc, nonnull));

MuonDatatypeOption *mu_datatype_option(MuonEngine *engine, MuonName *name)
  __attribute__((malloc, nonnull));

MuonDatatypeStmt *mu_datatype_stmt(
    MuonEngine *engine,
    MuonName *name,
    size_t argc,
    MuonDatatypeOption *argv[/* argc */])
  __attribute__((malloc, nonnull(1, 2)));

MuonDefineStmt *mu_define_stmt(
    MuonEngine *engine, MuonName *name, MuonExpr *expr)
  __attribute__((malloc, nonnull));

/// The header that each concrete view must have
#define MU_VIEW_HEADER union { \
  struct MuonView as_view; struct MuonNode as_node; \
}

typedef const struct MuonViewMember {
  MU_NODE_HEADER;
  size_t announce_length;
  MuonName *name; // optional
  MuonView *view;
} MuonViewMember;

typedef const struct MuonRecordView {
  MU_VIEW_HEADER;
  size_t announce_length;
  size_t argc;
  MuonViewMember *argv[/* argc */];
} MuonRecordView;

typedef const struct MuonVariableView {
  MU_VIEW_HEADER;
  MuonName *name;
} MuonVariableView;

MuonViewMember *mu_view_member(
    MuonEngine *engine, MuonName *name, MuonView *view)
  __attribute__((malloc, nonnull));

MuonRecordView *mu_record_view(
    MuonEngine *engine, size_t argc, MuonViewMember *argv[/* argc */])
  __attribute__((malloc, nonnull(1)));

MuonVariableView *mu_variable_view(MuonEngine *engine, MuonName *name)
  __attribute__((malloc, nonnull));

/// Emit debugging information on the abstract @a node to the debug stream
void mu_node_debug(MuonNode *node) __attribute__((nonnull));

/// @internal Used to emit each branch in MU_NODE_ENUMERATOR()
#define MU_NODE_ENUMERATOR_EMIT(l, upper, title) \
  , Muon##title *: MU_##upper##_NODE

/// Return the enumerator constant indicative of the concrete node @a type
#define MU_NODE_ENUMERATOR(type) \
  _Generic((type) {0} MU_EACH_NODE_KIND(MU_NODE_ENUMERATOR_EMIT))

/// @internal Used to emit each branch in mu_expr_cast()
#define MU_EXPR_CAST_EMIT(l, upper, title, ...) \
  , Muon##title##Expr *: _kind == MU_##upper##_EXPR##__VA_ARGS__

/// @internal Used to emit each branch in mu_sign_cast()
#define MU_SIGN_CAST_EMIT(l, upper, title, ...) \
  , Muon##title##Sign *: _kind == MU_##upper##_SIGN##__VA_ARGS__

/// @internal Used to emit each branch in mu_stmt_cast()
#define MU_STMT_CAST_EMIT(l, upper, title, ...) \
  , Muon##title##Stmt *: _kind == MU_##upper##_STMT##__VA_ARGS__

/// @internal Used to emit each branch in mu_view_cast()
#define MU_VIEW_CAST_EMIT(l, upper, title, ...) \
  , Muon##title##View *: _kind == MU_##upper##_VIEW##__VA_ARGS__

/**
 * @brief Downcast the @a abstract node to the <tt>typeof(concrete)</tt>
 *
 * @a abstract should have type <tt>MuonNode *</tt>. @a concrete should
 * be, or have, the type of a pointer to a const qualified concrete node, or:
 *
 * - <tt>MuonExpr *</tt>
 * - <tt>MuonSign *</tt>
 * - <tt>MuonStmt *</tt>
 * - <tt>MuonView *</tt>
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
 * - @a concrete isn't, or doesn't have, the type of:
 *   - <tt>MuonExpr *</tt>
 *   - <tt>MuonSign *</tt>
 *   - <tt>MuonStmt *</tt>
 *   - <tt>MuonView *</tt>
 *   - or a const qualified pointer to a concrete node
 */
#define mu_node_cast(abstract, concrete) __extension__ ({ \
  MuonNode *_abstract = (abstract); \
  __typeof__(concrete) _concrete; \
  _abstract->kind == MU_NODE_ENUMERATOR(__typeof__(_concrete)) ? \
    (__typeof__(_concrete)) _abstract : NULL; \
})

/**
 * @brief Downcast the @a abstract expr to the <tt>typeof(concrete)</tt>
 *
 * @a abstract should have type <tt>MuonExpr *</tt>. @a concrete should
 * be, or have, the type of a pointer to a const qualified concrete expr. Then
 * if @a abstract is an instance of that type, it will be cast to that type and
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
#define mu_expr_cast(abstract, concrete) __extension__ ({ \
    MuonExpr *_abstract = (abstract); \
    typeof(concrete) _concrete; \
    MuonExprKind _kind = _abstract->kind; \
    int _castable = _Generic(_concrete MU_EACH_EXPR_KIND(MU_EXPR_CAST_EMIT)); \
    _castable ? (typeof(_concrete)) _abstract : NULL; \
  })

/**
 * @brief Downcast the @a abstract sign to the <tt>typeof(concrete)</tt>
 *
 * @a abstract should have type <tt>MuonSign *</tt>. @a concrete should
 * be, or have, the type of a pointer to a const qualified concrete sign. Then
 * if @a abstract is an instance of that type, it will be cast to that type and
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
#define mu_sign_cast(abstract, concrete) __extension__ ({ \
    MuonSign *_abstract = (abstract); \
    typeof(concrete) _concrete; \
    MuonSignKind _kind = _abstract->kind; \
    int _castable = _Generic(_concrete MU_EACH_SIGN_KIND(MU_SIGN_CAST_EMIT)); \
    _castable ? (typeof(_concrete)) _abstract : NULL; \
  })

/**
 * @brief Downcast the @a abstract stmt to the <tt>typeof(concrete)</tt>
 *
 * @a abstract should have type <tt>MuonStmt *</tt>. @a concrete should
 * be, or have, the type of a pointer to a const qualified concrete stmt. Then
 * if @a abstract is an instance of that type, it will be cast to that type and
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
    int _castable = _Generic(_concrete MU_EACH_STMT_KIND(MU_STMT_CAST_EMIT)); \
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
    int _castable = _Generic(_concrete MU_EACH_VIEW_KIND(MU_VIEW_CAST_EMIT)); \
    _castable ? (typeof(_concrete)) _abstract : NULL; \
  })

#endif /* MU_ENGINE_NODE_H */
