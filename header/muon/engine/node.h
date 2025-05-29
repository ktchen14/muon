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
} mu_node_kind_t;

/// An enumeration over each kind of expr, e.g. @c MU_ACCESS_EXPR
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_EXPR = MU_##upper##_EXPR_NODE,
  MU_EACH_EXPR_KIND(MU_EMIT)
#undef MU_EMIT
} mu_expr_kind_t;

/// An enumeration over each kind of sign, e.g. @c MU_BOOLEAN_SIGN
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_SIGN = MU_##upper##_SIGN_NODE,
  MU_EACH_SIGN_KIND(MU_EMIT)
#undef MU_EMIT
} mu_sign_kind_t;

/// An enumeration over each kind of stmt, e.g. @c MU_DEFINE_STMT
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_STMT = MU_##upper##_STMT_NODE,
  MU_EACH_STMT_KIND(MU_EMIT)
#undef MU_EMIT
} mu_stmt_kind_t;

/// An enumeration over each kind of view, e.g. @c MU_VARIABLE_VIEW
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_VIEW = MU_##upper##_VIEW_NODE,
  MU_EACH_VIEW_KIND(MU_EMIT)
#undef MU_EMIT
} mu_view_kind_t;

/**
 * @brief An abstract node
 *
 * Note that a mu_node_t is a constant object; the mutable equivalent is a
 * struct mu_node_t.
 */
typedef const struct mu_node_t {
  mu_node_kind_t kind;
  const MuonEngine *engine;
  size_t id;
} mu_node_t;

/// The header that each concrete node must have
#define MU_NODE_HEADER struct mu_node_t as_node

/**
 * @brief An abstract expr
 *
 * Note that a mu_expr_t is a constant object; the mutable equivalent is a
 * struct mu_expr_t.
 */
typedef const struct mu_expr_t {
  union { MU_NODE_HEADER; mu_expr_kind_t kind; };
} mu_expr_t;

/**
 * @brief An abstract sign
 *
 * Note that a mu_sign_t is a constant object; the mutable equivalent is a
 * struct mu_sign_t.
 */
typedef const struct mu_sign_t {
  union { MU_NODE_HEADER; mu_sign_kind_t kind; };
} mu_sign_t;

/**
 * @brief An abstract stmt
 *
 * Note that a mu_stmt_t is a constant object; the mutable equivalent is a
 * struct mu_stmt_t.
 */
typedef const struct mu_stmt_t {
  union { MU_NODE_HEADER; mu_stmt_kind_t kind; };
} mu_stmt_t;

/**
 * @brief An abstract view
 *
 * Note that a mu_view_t is a constant object; the mutable equivalent is a
 * struct mu_view_t.
 */
typedef const struct mu_view_t {
  union { MU_NODE_HEADER; mu_view_kind_t kind; };
} mu_view_t;

/// The header that each concrete expr must have
#define MU_EXPR_HEADER union { \
  struct mu_expr_t as_expr; struct mu_node_t as_node; \
}

typedef const struct mu_access_expr_t {
  MU_EXPR_HEADER;
  MuonName *name;
} mu_access_expr_t;

typedef const struct mu_boolean_expr_t {
  MU_EXPR_HEADER;
  _Bool data;
} mu_boolean_expr_t;

typedef const struct mu_cast_expr_t {
  MU_EXPR_HEADER;
  mu_sign_t *sign;
  mu_expr_t *matter;
} mu_cast_expr_t;

typedef const struct mu_integer_expr_t {
  MU_EXPR_HEADER;
  uint64_t data;
} mu_integer_expr_t;

typedef const struct mu_invoke_expr_t {
  MU_EXPR_HEADER;
  mu_expr_t *operator;
  mu_expr_t *argument;
} mu_invoke_expr_t;

typedef const struct mu_lambda_expr_t {
  MU_EXPR_HEADER;
  mu_view_t *argument;
  mu_expr_t *matter;
} mu_lambda_expr_t;

typedef const struct mu_name_expr_t {
  MU_EXPR_HEADER;
  MuonName *name;
} mu_name_expr_t;

typedef const struct mu_native_expr_t {
  MU_EXPR_HEADER;
  MuonName *name;
} mu_native_expr_t;

typedef const struct mu_expr_member_t {
  MU_NODE_HEADER;
  MuonName *name; // optional
  mu_expr_t *expr;
} mu_expr_member_t;

typedef const struct mu_record_expr_t {
  MU_EXPR_HEADER;
  size_t argc;
  mu_expr_member_t *argv[/* argc */];
} mu_record_expr_t;

typedef const struct mu_sequence_expr_t {
  MU_EXPR_HEADER;
  size_t argc;
  mu_stmt_t *argv[/* argc */];
} mu_sequence_expr_t;

typedef const struct mu_switch_case_t {
  MU_NODE_HEADER;
  MuonName *name;
  mu_expr_t *expr;
} mu_switch_case_t;

typedef const struct mu_switch_expr_t {
  MU_EXPR_HEADER;
  size_t argc;
  mu_switch_case_t *argv[/* argc */];
} mu_switch_expr_t;

typedef const struct mu_vector_expr_t {
  MU_EXPR_HEADER;
  size_t argc;
  mu_expr_t *argv[/* argc */];
} mu_vector_expr_t;

mu_access_expr_t *mu_access_expr(MuonEngine *engine, MuonName *name)
  __attribute__((malloc, nonnull));

mu_boolean_expr_t *mu_boolean_expr(MuonEngine *engine, _Bool data)
  __attribute__((malloc, nonnull));

mu_cast_expr_t *mu_cast_expr(
    MuonEngine *engine, mu_sign_t *sign, mu_expr_t *matter)
  __attribute__((malloc, nonnull));

mu_integer_expr_t *mu_integer_expr(MuonEngine *engine, uint64_t data)
  __attribute__((malloc, nonnull));

mu_invoke_expr_t *mu_invoke_expr(
    MuonEngine *engine, mu_expr_t *operator, mu_expr_t *argument)
  __attribute__((malloc, nonnull));

mu_lambda_expr_t *mu_lambda_expr(
    MuonEngine *engine, mu_view_t *argument, mu_expr_t *matter)
  __attribute__((malloc, nonnull));

mu_name_expr_t *mu_name_expr(MuonEngine *engine, MuonName *name)
  __attribute__((malloc, nonnull));

mu_native_expr_t *mu_native_expr(MuonEngine *engine, MuonName *name)
  __attribute__((malloc, nonnull));

mu_expr_member_t *mu_expr_member(
    MuonEngine *engine, MuonName *name, mu_expr_t *expr)
  __attribute__((malloc, nonnull));

mu_record_expr_t *mu_record_expr(
    MuonEngine *engine, size_t argc, mu_expr_member_t *argv[/* argc */])
  __attribute__((malloc, nonnull(1)));

mu_switch_case_t *mu_switch_case(
    MuonEngine *engine, MuonName *name, mu_expr_t *expr)
  __attribute__((malloc, nonnull));

mu_switch_expr_t *mu_switch_expr(
    MuonEngine *engine, size_t argc, mu_switch_case_t *const argv[argc])
  __attribute__((malloc, nonnull));

mu_sequence_expr_t *mu_sequence_expr(
    MuonEngine *engine, size_t argc, mu_stmt_t *const argv[argc])
  __attribute__((malloc, nonnull));

mu_vector_expr_t *mu_vector_expr(
    MuonEngine *engine, size_t argc, mu_expr_t *const argv[/* argc */])
  __attribute__((malloc, nonnull(1)));

/// The header that each concrete sign must have
#define MU_SIGN_HEADER union { \
  struct mu_sign_t as_sign; struct mu_node_t as_node; \
}

typedef const struct mu_boolean_sign_t {
  MU_SIGN_HEADER;
} mu_boolean_sign_t;

typedef const struct mu_integer_sign_t {
  MU_SIGN_HEADER;
} mu_integer_sign_t;

typedef const struct mu_lambda_sign_t {
  MU_SIGN_HEADER;
  mu_sign_t *argument;
  mu_sign_t *output;
} mu_lambda_sign_t;

typedef const struct mu_name_sign_t {
  MU_SIGN_HEADER;
  MuonName *name;
} mu_name_sign_t;

typedef struct {
  MuonName *name; // optional
  mu_sign_t *sign;
} mu_sign_member_t;

typedef const struct mu_record_sign_t {
  MU_SIGN_HEADER;
  size_t argc;
  mu_sign_member_t argv[/* argc */];
} mu_record_sign_t;

typedef const struct mu_vector_sign_t {
  MU_SIGN_HEADER;
  mu_sign_t *matter;
} mu_vector_sign_t;

mu_boolean_sign_t *mu_boolean_sign(MuonEngine *engine)
  __attribute__((malloc, nonnull));

mu_integer_sign_t *mu_integer_sign(MuonEngine *engine)
  __attribute__((malloc, nonnull));

mu_lambda_sign_t *mu_lambda_sign(
    MuonEngine *engine, mu_sign_t *argument, mu_sign_t *output)
  __attribute__((malloc, nonnull));

mu_name_sign_t *mu_name_sign(MuonEngine *engine, MuonName *name)
  __attribute__((malloc, nonnull));

mu_record_sign_t *mu_record_sign(
    MuonEngine *engine, size_t argc, const mu_sign_member_t argv[/* argc */])
  __attribute__((malloc, nonnull(1)));

mu_vector_sign_t *mu_vector_sign(MuonEngine *engine, mu_sign_t *matter)
  __attribute__((malloc, nonnull));

/// The header that each concrete stmt must have
#define MU_STMT_HEADER union { \
  struct mu_stmt_t as_stmt; struct mu_node_t as_node; \
}

typedef const struct mu_coercion_stmt_t {
  MU_STMT_HEADER;
  mu_sign_t *source;
  mu_sign_t *target;
  mu_expr_t *expr;
} mu_coercion_stmt_t;

typedef const struct mu_type_node_t {
  MU_NODE_HEADER;
  MuonName *name;
  size_t argc;
  MuonName *argv[/* argc */];
} mu_type_node_t;

typedef const struct mu_datatype_option_t {
  MU_NODE_HEADER;
  MuonName *name;
} mu_datatype_option_t;

typedef const struct mu_datatype_stmt_t {
  MU_STMT_HEADER;
  MuonName *name;
  size_t argc;
  mu_datatype_option_t *argv[/* argc */];
} mu_datatype_stmt_t;

typedef const struct mu_define_stmt_t {
  MU_STMT_HEADER;
  MuonName *name;
  mu_expr_t *expr;
} mu_define_stmt_t;

mu_coercion_stmt_t *mu_coercion_stmt(
    MuonEngine *engine, mu_sign_t *source, mu_sign_t *target, mu_expr_t *expr)
  __attribute__((malloc, nonnull));

mu_datatype_option_t *mu_datatype_option(MuonEngine *engine, MuonName *name)
  __attribute__((malloc, nonnull));

mu_datatype_stmt_t *mu_datatype_stmt(
    MuonEngine *engine,
    MuonName *name,
    size_t argc,
    mu_datatype_option_t *argv[/* argc */])
  __attribute__((malloc, nonnull(1, 2)));

mu_define_stmt_t *mu_define_stmt(
    MuonEngine *engine, MuonName *name, mu_expr_t *expr)
  __attribute__((malloc, nonnull));

/// The header that each concrete view must have
#define MU_VIEW_HEADER union { \
  struct mu_view_t as_view; struct mu_node_t as_node; \
}

typedef const struct mu_view_member_t {
  MU_NODE_HEADER;
  size_t announce_length;
  MuonName *name; // optional
  mu_view_t *view;
} mu_view_member_t;

typedef const struct mu_record_view_t {
  MU_VIEW_HEADER;
  size_t announce_length;
  size_t argc;
  mu_view_member_t *argv[/* argc */];
} mu_record_view_t;

typedef const struct mu_variable_view_t {
  MU_VIEW_HEADER;
  MuonName *name;
} mu_variable_view_t;

mu_view_member_t *mu_view_member(
    MuonEngine *engine, MuonName *name, mu_view_t *view)
  __attribute__((malloc, nonnull));

mu_record_view_t *mu_record_view(
    MuonEngine *engine, size_t argc, mu_view_member_t *argv[/* argc */])
  __attribute__((malloc, nonnull(1)));

mu_variable_view_t *mu_variable_view(MuonEngine *engine, MuonName *name)
  __attribute__((malloc, nonnull));

/// Emit debugging information on the abstract @a node to the debug stream
void mu_node_debug(mu_node_t *node) __attribute__((nonnull));

/// @internal Used to emit each branch in MU_NODE_ENUMERATOR()
#define MU_NODE_ENUMERATOR_EMIT(lower, upper, t) \
  , mu_##lower##_t *: MU_##upper##_NODE

/// Return the enumerator constant indicative of the concrete node @a type
#define MU_NODE_ENUMERATOR(type) \
  _Generic((type) {0} MU_EACH_NODE_KIND(MU_NODE_ENUMERATOR_EMIT))

/// @internal Used to emit each branch in mu_expr_cast()
#define MU_EXPR_CAST_EMIT(lower, upper, t, ...) \
  , mu_##lower##_expr_t *: _kind == MU_##upper##_EXPR##__VA_ARGS__

/// @internal Used to emit each branch in mu_sign_cast()
#define MU_SIGN_CAST_EMIT(lower, upper, t, ...) \
  , mu_##lower##_sign_t *: _kind == MU_##upper##_SIGN##__VA_ARGS__

/// @internal Used to emit each branch in mu_stmt_cast()
#define MU_STMT_CAST_EMIT(lower, upper, t, ...) \
  , mu_##lower##_stmt_t *: _kind == MU_##upper##_STMT##__VA_ARGS__

/// @internal Used to emit each branch in mu_view_cast()
#define MU_VIEW_CAST_EMIT(lower, upper, t, ...) \
  , mu_##lower##_view_t *: _kind == MU_##upper##_VIEW##__VA_ARGS__

/**
 * @brief Downcast the @a abstract node to the <tt>typeof(concrete)</tt>
 *
 * @a abstract should have type <tt>mu_node_t *</tt>. @a concrete should
 * be, or have, the type of a pointer to a const qualified concrete node, or:
 *
 * - <tt>mu_expr_t *</tt>
 * - <tt>mu_sign_t *</tt>
 * - <tt>mu_stmt_t *</tt>
 * - <tt>mu_view_t *</tt>
 *
 * Then if @a abstract is an instance of that type, it will be cast to that type
 * and returned. Otherwise, this will return @c NULL.
 *
 * @par Example:
 * @code{.c}
 *   mu_node_t *abstract_node = ...;
 *
 *   mu_access_expr_t *expr;
 *   if ((expr = mu_expr_cast(abstract_node, expr)) == NULL)
 *     return ...;
 * @endcode
 *
 * The behavior is undefined if:
 * - @a abstract is @c NULL
 * - @a abstract doesn't have type <tt>mu_node_t *</tt>
 * - @a concrete isn't, or doesn't have, the type of:
 *   - <tt>mu_expr_t *</tt>
 *   - <tt>mu_sign_t *</tt>
 *   - <tt>mu_stmt_t *</tt>
 *   - <tt>mu_view_t *</tt>
 *   - or a const qualified pointer to a concrete node
 */
#define mu_node_cast(abstract, concrete) __extension__ ({ \
  mu_node_t *_abstract = (abstract); \
  __typeof__(concrete) _concrete; \
  _abstract->kind == MU_NODE_ENUMERATOR(__typeof__(_concrete)) ? \
    (__typeof__(_concrete)) _abstract : NULL; \
})

/**
 * @brief Downcast the @a abstract expr to the <tt>typeof(concrete)</tt>
 *
 * @a abstract should have type <tt>mu_expr_t *</tt>. @a concrete should
 * be, or have, the type of a pointer to a const qualified concrete expr. Then
 * if @a abstract is an instance of that type, it will be cast to that type and
 * returned. Otherwise, this will return @c NULL.
 *
 * @par Example:
 * @code{.c}
 *   mu_expr_t *abstract_expr = ...;
 *
 *   mu_access_expr_t *expr;
 *   if ((expr = mu_expr_cast(abstract_expr, expr)) == NULL)
 *     return ...;
 * @endcode
 *
 * The behavior is undefined if:
 * - @a abstract is @c NULL
 * - @a abstract doesn't have type <tt>mu_expr_t *</tt>
 * - @a concrete isn't, or doesn't have, the type of a const qualified pointer
 *   to a concrete expr
 */
#define mu_expr_cast(abstract, concrete) __extension__ ({ \
    mu_expr_t *_abstract = (abstract); \
    typeof(concrete) _concrete; \
    mu_expr_kind_t _kind = _abstract->kind; \
    int _castable = _Generic(_concrete MU_EACH_EXPR_KIND(MU_EXPR_CAST_EMIT)); \
    _castable ? (typeof(_concrete)) _abstract : NULL; \
  })

/**
 * @brief Downcast the @a abstract sign to the <tt>typeof(concrete)</tt>
 *
 * @a abstract should have type <tt>mu_sign_t *</tt>. @a concrete should
 * be, or have, the type of a pointer to a const qualified concrete sign. Then
 * if @a abstract is an instance of that type, it will be cast to that type and
 * returned. Otherwise, this will return @c NULL.
 *
 * @par Example:
 * @code{.c}
 *   mu_sign_t *abstract_sign = ...;
 *
 *   mu_vector_sign_t *sign;
 *   if ((sign = mu_sign_cast(abstract_sign, sign)) == NULL)
 *     return ...;
 * @endcode
 *
 * The behavior is undefined if:
 * - @a abstract is @c NULL
 * - @a abstract doesn't have type <tt>mu_sign_t *</tt>
 * - @a concrete isn't, or doesn't have, the type of a const qualified pointer
 *   to a concrete sign
 */
#define mu_sign_cast(abstract, concrete) __extension__ ({ \
    mu_sign_t *_abstract = (abstract); \
    typeof(concrete) _concrete; \
    mu_sign_kind_t _kind = _abstract->kind; \
    int _castable = _Generic(_concrete MU_EACH_SIGN_KIND(MU_SIGN_CAST_EMIT)); \
    _castable ? (typeof(_concrete)) _abstract : NULL; \
  })

/**
 * @brief Downcast the @a abstract stmt to the <tt>typeof(concrete)</tt>
 *
 * @a abstract should have type <tt>mu_stmt_t *</tt>. @a concrete should
 * be, or have, the type of a pointer to a const qualified concrete stmt. Then
 * if @a abstract is an instance of that type, it will be cast to that type and
 * returned. Otherwise, this will return @c NULL.
 *
 * @par Example:
 * @code{.c}
 *   mu_stmt_t *abstract_stmt = ...;
 *
 *   mu_vector_stmt_t *stmt;
 *   if ((stmt = mu_stmt_cast(abstract_stmt, stmt)) == NULL)
 *     return ...;
 * @endcode
 *
 * The behavior is undefined if:
 * - @a abstract is @c NULL
 * - @a abstract doesn't have type <tt>mu_stmt_t *</tt>
 * - @a concrete isn't, or doesn't have, the type of a const qualified pointer
 *   to a concrete stmt
 */
#define mu_stmt_cast(abstract, concrete) __extension__ ({ \
    mu_stmt_t *_abstract = (abstract); \
    typeof(concrete) _concrete; \
    mu_stmt_kind_t _kind = _abstract->kind; \
    int _castable = _Generic(_concrete MU_EACH_STMT_KIND(MU_STMT_CAST_EMIT)); \
    _castable ? (typeof(_concrete)) _abstract : NULL; \
  })

/**
 * @brief Downcast the @a abstract view to the <tt>typeof(concrete)</tt>
 *
 * @a abstract should have type <tt>mu_view_t *</tt>. @a concrete should
 * be, or have, the type of a pointer to a const qualified concrete view. Then
 * if @a abstract is an instance of that type, it will be cast to that type and
 * returned. Otherwise, this will return @c NULL.
 *
 * @par Example:
 * @code{.c}
 *   mu_view_t *abstract_view = ...;
 *
 *   mu_vector_view_t *view;
 *   if ((view = mu_view_cast(abstract_view, view)) == NULL)
 *     return ...;
 * @endcode
 *
 * The behavior is undefined if:
 * - @a abstract is @c NULL
 * - @a abstract doesn't have type <tt>mu_view_t *</tt>
 * - @a concrete isn't, or doesn't have, the type of a const qualified pointer
 *   to a concrete view
 */
#define mu_view_cast(abstract, concrete) __extension__ ({ \
    mu_view_t *_abstract = (abstract); \
    typeof(concrete) _concrete; \
    mu_view_kind_t _kind = _abstract->kind; \
    int _castable = _Generic(_concrete MU_EACH_VIEW_KIND(MU_VIEW_CAST_EMIT)); \
    _castable ? (typeof(_concrete)) _abstract : NULL; \
  })

#endif /* MU_ENGINE_NODE_H */
