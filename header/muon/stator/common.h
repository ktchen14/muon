#ifndef MU_STATOR_COMMON_H
#define MU_STATOR_COMMON_H

#include <stddef.h>

/// Expands to emit(lower, upper, title, ...) for each kind of expr
#define MU_EACH_EXPR_KIND(emit, ...) \
  emit(access, ACCESS, Access, ##__VA_ARGS__) \
  emit(boolean, BOOLEAN, Boolean, ##__VA_ARGS__) \
  emit(integer, INTEGER, Integer, ##__VA_ARGS__) \
  emit(invoke, INVOKE, Invoke, ##__VA_ARGS__) \
  emit(lambda, LAMBDA, Lambda, ##__VA_ARGS__) \
  emit(name, NAME, Name, ##__VA_ARGS__) \
  emit(native, NATIVE, Native, ##__VA_ARGS__) \
  emit(record, RECORD, Record, ##__VA_ARGS__) \
  emit(sequence, SEQUENCE, Sequence, ##__VA_ARGS__) \
  emit(vector, VECTOR, Vector, ##__VA_ARGS__) \
  emit(zero, ZERO, Zero, ##__VA_ARGS__)

/// Expands to emit(lower, upper, title, ...) for each kind of sign
#define MU_EACH_SIGN_KIND(emit, ...) \
  emit(boolean, BOOLEAN, Boolean, ##__VA_ARGS__) \
  emit(integer, INTEGER, Integer, ##__VA_ARGS__) \
  emit(name, NAME, Name, ##__VA_ARGS__) \
  emit(record, RECORD, Record, ##__VA_ARGS__) \
  emit(vector, VECTOR, Vector, ##__VA_ARGS__)

/// Expands to emit(lower, upper, title, ...) for each kind of stmt
#define MU_EACH_STMT_KIND(emit, ...) \
  emit(define, DEFINE, Define, ##__VA_ARGS__) \
  emit(type, TYPE, Type, ##__VA_ARGS__)

/// Expands to emit(lower, upper, title, ...) for each kind of view
#define MU_EACH_VIEW_KIND(emit, ...) \
  emit(variable, VARIABLE, Variable, ##__VA_ARGS__)

/// Expands to emit(lower, upper, title, ...) for each kind of node
#define MU_EACH_NODE_KIND(emit, ...) \
  MU_EACH_EXPR_KIND(MU_EACH_NODE_EMIT, _expr, _EXPR, Expr, emit, ##__VA_ARGS__) \
  MU_EACH_SIGN_KIND(MU_EACH_NODE_EMIT, _sign, _SIGN, Sign, emit, ##__VA_ARGS__) \
  MU_EACH_STMT_KIND(MU_EACH_NODE_EMIT, _stmt, _STMT, Stmt, emit, ##__VA_ARGS__) \
  MU_EACH_VIEW_KIND(MU_EACH_NODE_EMIT, _view, _VIEW, View, emit, ##__VA_ARGS__) \
  emit(expr_member, EXPR_MEMBER, ExprMember, ##__VA_ARGS__)

/// @internal Used as @c emit in MU_EACH_NODE_KIND
#define MU_EACH_NODE_EMIT(l, u, t, lsuffix, usuffix, tsuffix, emit, ...) \
  emit(l##lsuffix, u##usuffix, t##tsuffix, ##__VA_ARGS__)

/// An enumeration over each kind of node, e.g. @c MU_ACCESS_EXPR_NODE
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_NODE,
  MU_EACH_NODE_KIND(MU_EMIT)
#undef MU_EMIT
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

typedef struct mu_engine_t mu_engine_t;

/// An abstract node
typedef struct mu_node_t {
  mu_node_kind_t kind;
  const mu_engine_t *engine;
  size_t id;
} mu_node_t;

/// The header that each concrete expr must have
#define MU_NODE_HEADER mu_node_t as_node

/// An abstract expr
typedef struct {
  union { mu_expr_kind_t kind; mu_node_t as_node; };
} mu_expr_t;

/// An abstract sign
typedef struct {
  union { mu_sign_kind_t kind; mu_node_t as_node; };
} mu_sign_t;

/// An abstract stmt
typedef struct {
  union { mu_stmt_kind_t kind; mu_node_t as_node; };
} mu_stmt_t;

/// An abstract view
typedef struct {
  union { mu_view_kind_t kind; mu_node_t as_node; };
} mu_view_t;

#endif /* MU_STATOR_COMMON_H */
