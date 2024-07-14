#ifndef MU_STATOR_COMMON_H
#define MU_STATOR_COMMON_H

#include <stddef.h>

/// Expands to emit(lower, upper, title, ...) for each kind of expr
#define MU_EACH_EXPR_KIND(emit, ...) \
  emit(access, ACCESS, Access, ##__VA_ARGS__) \
  emit(boolean, BOOLEAN, Boolean, ##__VA_ARGS__) \
  emit(integer, INTEGER, Integer, ##__VA_ARGS__) \
  emit(member, MEMBER, Member, ##__VA_ARGS__) \
  emit(name, NAME, Name, ##__VA_ARGS__) \
  emit(record, RECORD, Record, ##__VA_ARGS__) \
  emit(vector, VECTOR, Vector, ##__VA_ARGS__) \
  emit(zero, ZERO, Zero, ##__VA_ARGS__)

/// Expands to emit(lower, upper, title, ...) for each kind of sign
#define MU_EACH_SIGN_KIND(emit, ...) \
  emit(boolean, BOOLEAN, Boolean, ##__VA_ARGS__) \
  emit(integer, INTEGER, Integer, ##__VA_ARGS__) \
  emit(member, MEMBER, Member, ##__VA_ARGS__) \
  emit(name, NAME, Name, ##__VA_ARGS__) \
  emit(record, RECORD, Record, ##__VA_ARGS__) \
  emit(variable, VARIABLE, Variable, ##__VA_ARGS__) \
  emit(vector, VECTOR, Vector, ##__VA_ARGS__)

/// Expands to emit(lower, upper, title, ...) for each kind of stmt
#define MU_EACH_STMT_KIND(emit, ...) \
  emit(constant, CONSTANT, Constant, ##__VA_ARGS__) \
  emit(type, TYPE, Type, ##__VA_ARGS__)

/// Expands to emit(lower, upper, title, ...) for each kind of type
#define MU_EACH_TYPE_KIND(emit, ...) \
  emit(boolean, BOOLEAN, Boolean, ##__VA_ARGS__) \
  emit(integer, INTEGER, Integer, ##__VA_ARGS__) \
  emit(variable, VARIABLE, Variable, ##__VA_ARGS__) \
  emit(vector, VECTOR, Vector, ##__VA_ARGS__)

/// @internal Used to emit each abstract branch in a cast
#define MU_CAST_EMIT(l, upper, t, ...) || _kind == MU_##upper##__VA_ARGS__

/**
 * @brief An enumeration of each kind of stator
 */
typedef enum {
  MU_NAME_STATOR,

#define MU_EMIT(l, upper, t, kind) MU_##upper##_##kind##_STATOR,
  MU_EACH_EXPR_KIND(MU_EMIT, EXPR)
  MU_EACH_SIGN_KIND(MU_EMIT, SIGN)
  MU_EACH_STMT_KIND(MU_EMIT, STMT)

  MU_EACH_TYPE_KIND(MU_EMIT, TYPE)
#undef MU_EMIT
} mu_stator_kind_t;

typedef struct mu_engine_t mu_engine_t;

/// An abstract stator
typedef struct {
  mu_stator_kind_t kind;
  const mu_engine_t *engine;
  size_t id;
} mu_stator_t;

/// The header that each concrete stator must have
#define MU_STATOR_HEADER mu_stator_t as_stator

#endif /* MU_STATOR_COMMON_H */
