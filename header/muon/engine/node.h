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

/// An abstract node
typedef struct mu_node_t {
  mu_node_kind_t kind;
  const mu_engine_t *engine;
  size_t id;
} mu_node_t;

/// The header that each concrete node must have
#define MU_NODE_HEADER mu_node_t as_node

/// An abstract expr
typedef struct mu_expr_t mu_expr_t;
struct mu_expr_t {
  union { MU_NODE_HEADER; mu_expr_kind_t kind; };
};

/// An abstract sign
typedef struct {
  union { MU_NODE_HEADER; mu_sign_kind_t kind; };
} mu_sign_t;

/// An abstract stmt
typedef struct {
  union { MU_NODE_HEADER; mu_stmt_kind_t kind; };
} mu_stmt_t;

/// An abstract view
typedef struct {
  union { MU_NODE_HEADER; mu_view_kind_t kind; };
} mu_view_t;

/// The header that each concrete expr must have
#define MU_EXPR_HEADER union { mu_expr_t as_expr; mu_node_t as_node; }

typedef struct {
  MU_EXPR_HEADER;
  const mu_name_t *name;
} mu_access_expr_t;

typedef struct {
  MU_EXPR_HEADER;
  _Bool data;
} mu_boolean_expr_t;

typedef struct {
  MU_EXPR_HEADER;
  const mu_sign_t *sign;
  const mu_expr_t *matter;
} mu_cast_expr_t;

typedef struct {
  MU_EXPR_HEADER;
  uint64_t data;
} mu_integer_expr_t;

typedef struct {
  MU_EXPR_HEADER;
  const mu_expr_t *operator;
  const mu_expr_t *argument;
} mu_invoke_expr_t;

typedef struct {
  MU_EXPR_HEADER;
  const mu_view_t *argument;
  const mu_expr_t *matter;
} mu_lambda_expr_t;

typedef struct {
  MU_EXPR_HEADER;
  const mu_name_t *name;
} mu_name_expr_t;

typedef struct {
  MU_EXPR_HEADER;
  const mu_name_t *name;
} mu_native_expr_t;

typedef struct {
  MU_NODE_HEADER;
  const mu_name_t *name; // optional
  const mu_expr_t *expr;
} mu_expr_member_t;

typedef struct {
  MU_EXPR_HEADER;
  size_t argc;
  const mu_expr_member_t *argv[/* argc */];
} mu_record_expr_t;

typedef struct {
  MU_EXPR_HEADER;
  size_t argc;
  const mu_stmt_t *argv[/* argc */];
} mu_sequence_expr_t;

typedef struct {
  MU_NODE_HEADER;
  const mu_name_t *name;
  const mu_expr_t *expr;
} mu_switch_case_t;

typedef struct {
  MU_EXPR_HEADER;
  size_t argc;
  const mu_switch_case_t *argv[/* argc */];
} mu_switch_expr_t;

typedef struct {
  MU_EXPR_HEADER;
  size_t argc;
  const mu_expr_t *argv[/* argc */];
} mu_vector_expr_t;

const mu_access_expr_t *mu_access_expr(
    mu_engine_t *engine, const mu_name_t *name)
  __attribute__((malloc, nonnull));

const mu_boolean_expr_t *mu_boolean_expr(mu_engine_t *engine, _Bool data)
  __attribute__((malloc, nonnull));

const mu_cast_expr_t *mu_cast_expr(
    mu_engine_t *engine, const mu_sign_t *sign, const mu_expr_t *matter)
  __attribute__((malloc, nonnull));

const mu_integer_expr_t *mu_integer_expr(mu_engine_t *engine, uint64_t data)
  __attribute__((malloc, nonnull));

const mu_invoke_expr_t *mu_invoke_expr(
    mu_engine_t *engine, const mu_expr_t *operator, const mu_expr_t *argument)
  __attribute__((malloc, nonnull));

const mu_lambda_expr_t *mu_lambda_expr(
    mu_engine_t *engine, const mu_view_t *argument, const mu_expr_t *matter)
  __attribute__((malloc, nonnull));

const mu_name_expr_t *mu_name_expr(mu_engine_t *engine, const mu_name_t *name)
  __attribute__((malloc, nonnull));

const mu_native_expr_t *mu_native_expr(
    mu_engine_t *engine, const mu_name_t *name)
  __attribute__((malloc, nonnull));

const mu_expr_member_t *mu_expr_member(
    mu_engine_t *engine, const mu_name_t *name, const mu_expr_t *expr)
  __attribute__((malloc, nonnull));

const mu_record_expr_t *mu_record_expr(
    mu_engine_t *engine, size_t argc, const mu_expr_member_t *argv[/* argc */])
  __attribute__((malloc, nonnull(1)));

const mu_switch_case_t *mu_switch_case(
    mu_engine_t *engine, const mu_name_t *name, const mu_expr_t *expr)
  __attribute__((malloc, nonnull));

const mu_switch_expr_t *mu_switch_expr(
    mu_engine_t *engine, size_t argc, const mu_switch_case_t *const argv[argc])
  __attribute__((malloc, nonnull));

const mu_sequence_expr_t *mu_sequence_expr(
    mu_engine_t *engine, size_t argc, const mu_stmt_t *const argv[argc])
  __attribute__((malloc, nonnull));

const mu_vector_expr_t *mu_vector_expr(
    mu_engine_t *engine, size_t argc, const mu_expr_t *const argv[/* argc */])
  __attribute__((malloc, nonnull(1)));

/// The header that each concrete sign must have
#define MU_SIGN_HEADER union { mu_sign_t as_sign; mu_node_t as_node; }

typedef struct {
  MU_SIGN_HEADER;
} mu_boolean_sign_t;

typedef struct {
  MU_SIGN_HEADER;
} mu_integer_sign_t;

typedef struct {
  MU_SIGN_HEADER;
  const mu_sign_t *argument;
  const mu_sign_t *output;
} mu_lambda_sign_t;

typedef struct {
  MU_SIGN_HEADER;
  const mu_name_t *name;
} mu_name_sign_t;

typedef struct {
  const mu_name_t *name; // optional
  const mu_sign_t *sign;
} mu_sign_member_t;

typedef struct {
  MU_SIGN_HEADER;
  size_t argc;
  mu_sign_member_t argv[/* argc */];
} mu_record_sign_t;

typedef struct {
  MU_SIGN_HEADER;
  const mu_sign_t *matter;
} mu_vector_sign_t;

const mu_boolean_sign_t *mu_boolean_sign(mu_engine_t *engine)
  __attribute__((malloc, nonnull));

const mu_integer_sign_t *mu_integer_sign(mu_engine_t *engine)
  __attribute__((malloc, nonnull));

const mu_lambda_sign_t *mu_lambda_sign(
    mu_engine_t *engine, const mu_sign_t *argument, const mu_sign_t *output)
  __attribute__((malloc, nonnull));

const mu_name_sign_t *mu_name_sign(mu_engine_t *engine, const mu_name_t *name)
  __attribute__((malloc, nonnull));

const mu_record_sign_t *mu_record_sign(
    mu_engine_t *engine, size_t argc, const mu_sign_member_t argv[/* argc */])
  __attribute__((malloc, nonnull(1)));

const mu_vector_sign_t *mu_vector_sign(
    mu_engine_t *engine, const mu_sign_t *matter)
  __attribute__((malloc, nonnull));

/// The header that each concrete stmt must have
#define MU_STMT_HEADER union { mu_stmt_t as_stmt; mu_node_t as_node; }

typedef struct {
  MU_STMT_HEADER;
  const mu_sign_t *source;
  const mu_sign_t *target;
  const mu_expr_t *expr;
} mu_coercion_stmt_t;

typedef struct {
  MU_NODE_HEADER;
  const mu_name_t *name;
  size_t argc;
  const mu_name_t *argv[/* argc */];
} mu_type_node_t;

typedef struct {
  MU_NODE_HEADER;
  const mu_name_t *name;
} mu_datatype_option_t;

typedef struct {
  MU_STMT_HEADER;
  const mu_name_t *name;
  size_t argc;
  const mu_datatype_option_t *argv[/* argc */];
} mu_datatype_stmt_t;

typedef struct {
  MU_STMT_HEADER;
  const mu_name_t *name;
  const mu_expr_t *expr;
} mu_define_stmt_t;

const mu_coercion_stmt_t *mu_coercion_stmt(
    mu_engine_t *engine,
    const mu_sign_t *source,
    const mu_sign_t *target,
    const mu_expr_t *expr)
  __attribute__((malloc, nonnull));

const mu_datatype_option_t *mu_datatype_option(
    mu_engine_t *engine, const mu_name_t *name)
  __attribute__((malloc, nonnull));

const mu_datatype_stmt_t *mu_datatype_stmt(
    mu_engine_t *engine,
    const mu_name_t *name,
    size_t argc,
    const mu_datatype_option_t *argv[/* argc */])
  __attribute__((malloc, nonnull(1, 2)));

const mu_define_stmt_t *mu_define_stmt(
    mu_engine_t *engine, const mu_name_t *name, const mu_expr_t *expr)
  __attribute__((malloc, nonnull));

/// The header that each concrete view must have
#define MU_VIEW_HEADER union { mu_view_t as_view; mu_node_t as_node; }

typedef struct {
  MU_NODE_HEADER;
  size_t announce_length;
  const mu_name_t *name; // optional
  const mu_view_t *view;
} mu_view_member_t;

typedef struct {
  MU_VIEW_HEADER;
  size_t announce_length;
  size_t argc;
  const mu_view_member_t *argv[/* argc */];
} mu_record_view_t;

typedef struct {
  MU_VIEW_HEADER;
  const mu_name_t *name;
} mu_variable_view_t;

const mu_view_member_t *mu_view_member(
    mu_engine_t *engine, const mu_name_t *name, const mu_view_t *view)
  __attribute__((malloc, nonnull));

const mu_record_view_t *mu_record_view(
    mu_engine_t *engine, size_t argc, const mu_view_member_t *argv[/* argc */])
  __attribute__((malloc, nonnull(1)));

const mu_variable_view_t *mu_variable_view(
    mu_engine_t *engine, const mu_name_t *name)
  __attribute__((malloc, nonnull));

/// Emit debugging information on the abstract @a node to the debug stream
void mu_node_debug(const mu_node_t *node) __attribute__((nonnull));

/// @internal Used to emit each abstract branch in a cast
#define MU_NODE_CAST_EMIT(l, upper, t, ...) || _kind == MU_##upper##__VA_ARGS__

/// @internal Used to emit each branch in mu_expr_cast()
#define MU_EXPR_CAST_EMIT(lower, upper, t, ...) \
  , const mu_##lower##_expr_t *: _kind == MU_##upper##_EXPR##__VA_ARGS__

/// @internal Used to emit each branch in mu_sign_cast()
#define MU_SIGN_CAST_EMIT(lower, upper, t, ...) \
  , const mu_##lower##_sign_t *: _kind == MU_##upper##_SIGN##__VA_ARGS__

/// @internal Used to emit each branch in mu_stmt_cast()
#define MU_STMT_CAST_EMIT(lower, upper, t, ...) \
  , const mu_##lower##_stmt_t *: _kind == MU_##upper##_STMT##__VA_ARGS__

/// @internal Used to emit each branch in mu_view_cast()
#define MU_VIEW_CAST_EMIT(lower, upper, t, ...) \
  , const mu_##lower##_view_t *: _kind == MU_##upper##_VIEW##__VA_ARGS__

/**
 * @brief Downcast the @a abstract node to the <tt>typeof(concrete)</tt>
 *
 * @a abstract should have type <tt>const mu_node_t *</tt>. @a concrete should
 * be, or have, the type of a pointer to a const qualified concrete node, or:
 *
 * - <tt>const mu_expr_t *</tt>
 * - <tt>const mu_sign_t *</tt>
 * - <tt>const mu_stmt_t *</tt>
 * - <tt>const mu_view_t *</tt>
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
 * - @a abstract doesn't have type <tt>const mu_node_t *</tt>
 * - @a concrete isn't, or doesn't have, the type of:
 *   - <tt>const mu_expr_t *</tt>
 *   - <tt>const mu_sign_t *</tt>
 *   - <tt>const mu_stmt_t *</tt>
 *   - <tt>const mu_view_t *</tt>
 *   - or a const qualified pointer to a concrete node
 */
#define mu_node_cast(abstract, concrete) __extension__ ({ \
    const mu_node_t *_abstract = (abstract); \
    typeof(concrete) _concrete; \
    mu_node_kind_t _kind = _abstract->kind; \
    _Bool _castable = _Generic(_concrete, \
      const mu_expr_t *: 0 MU_EACH_EXPR_KIND(MU_NODE_CAST_EMIT, _EXPR_NODE), \
      const mu_sign_t *: 0 MU_EACH_SIGN_KIND(MU_NODE_CAST_EMIT, _SIGN_NODE), \
      const mu_stmt_t *: 0 MU_EACH_STMT_KIND(MU_NODE_CAST_EMIT, _STMT_NODE), \
      const mu_view_t *: 0 MU_EACH_VIEW_KIND(MU_NODE_CAST_EMIT, _VIEW_NODE) \
      MU_EACH_EXPR_KIND(MU_EXPR_CAST_EMIT, _NODE) \
      MU_EACH_SIGN_KIND(MU_SIGN_CAST_EMIT, _NODE) \
      MU_EACH_STMT_KIND(MU_STMT_CAST_EMIT, _NODE) \
      MU_EACH_VIEW_KIND(MU_VIEW_CAST_EMIT, _NODE), \
      const mu_expr_member_t *: _kind == MU_EXPR_MEMBER_NODE, \
      const mu_switch_case_t *: _kind == MU_SWITCH_CASE_NODE); \
    _castable ? (typeof(_concrete)) _abstract : NULL; \
  })

/**
 * @brief Downcast the @a abstract expr to the <tt>typeof(concrete)</tt>
 *
 * @a abstract should have type <tt>const mu_expr_t *</tt>. @a concrete should
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
 * - @a abstract doesn't have type <tt>const mu_expr_t *</tt>
 * - @a concrete isn't, or doesn't have, the type of a const qualified pointer
 *   to a concrete expr
 */
#define mu_expr_cast(abstract, concrete) __extension__ ({ \
    const mu_expr_t *_abstract = (abstract); \
    typeof(concrete) _concrete; \
    mu_expr_kind_t _kind = _abstract->kind; \
    int _castable = _Generic(_concrete MU_EACH_EXPR_KIND(MU_EXPR_CAST_EMIT)); \
    _castable ? (typeof(_concrete)) _abstract : NULL; \
  })

/**
 * @brief Downcast the @a abstract sign to the <tt>typeof(concrete)</tt>
 *
 * @a abstract should have type <tt>const mu_sign_t *</tt>. @a concrete should
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
 * - @a abstract doesn't have type <tt>const mu_sign_t *</tt>
 * - @a concrete isn't, or doesn't have, the type of a const qualified pointer
 *   to a concrete sign
 */
#define mu_sign_cast(abstract, concrete) __extension__ ({ \
    const mu_sign_t *_abstract = (abstract); \
    typeof(concrete) _concrete; \
    mu_sign_kind_t _kind = _abstract->kind; \
    int _castable = _Generic(_concrete MU_EACH_SIGN_KIND(MU_SIGN_CAST_EMIT)); \
    _castable ? (typeof(_concrete)) _abstract : NULL; \
  })

/**
 * @brief Downcast the @a abstract stmt to the <tt>typeof(concrete)</tt>
 *
 * @a abstract should have type <tt>const mu_stmt_t *</tt>. @a concrete should
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
 * - @a abstract doesn't have type <tt>const mu_stmt_t *</tt>
 * - @a concrete isn't, or doesn't have, the type of a const qualified pointer
 *   to a concrete stmt
 */
#define mu_stmt_cast(abstract, concrete) __extension__ ({ \
    const mu_stmt_t *_abstract = (abstract); \
    typeof(concrete) _concrete; \
    mu_stmt_kind_t _kind = _abstract->kind; \
    int _castable = _Generic(_concrete MU_EACH_STMT_KIND(MU_STMT_CAST_EMIT)); \
    _castable ? (typeof(_concrete)) _abstract : NULL; \
  })

/**
 * @brief Downcast the @a abstract view to the <tt>typeof(concrete)</tt>
 *
 * @a abstract should have type <tt>const mu_view_t *</tt>. @a concrete should
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
 * - @a abstract doesn't have type <tt>const mu_view_t *</tt>
 * - @a concrete isn't, or doesn't have, the type of a const qualified pointer
 *   to a concrete view
 */
#define mu_view_cast(abstract, concrete) __extension__ ({ \
    const mu_view_t *_abstract = (abstract); \
    typeof(concrete) _concrete; \
    mu_view_kind_t _kind = _abstract->kind; \
    int _castable = _Generic(_concrete MU_EACH_VIEW_KIND(MU_VIEW_CAST_EMIT)); \
    _castable ? (typeof(_concrete)) _abstract : NULL; \
  })

#endif /* MU_ENGINE_NODE_H */
