#ifndef MU_STATOR_H
#define MU_STATOR_H

#include "stator/common.h"         // IWYU pragma: export
#include "stator/name.h"           // IWYU pragma: export
#include "stator/node.h"           // IWYU pragma: export
#include "stator/type.h"           // IWYU pragma: export

#include "stator/access_expr.h"    // IWYU pragma: export
#include "stator/boolean_expr.h"   // IWYU pragma: export
#include "stator/integer_expr.h"   // IWYU pragma: export
#include "stator/member_expr.h"    // IWYU pragma: export
#include "stator/name_expr.h"      // IWYU pragma: export
#include "stator/record_expr.h"    // IWYU pragma: export
#include "stator/vector_expr.h"    // IWYU pragma: export
#include "stator/zero_expr.h"      // IWYU pragma: export

#include "stator/boolean_sign.h"   // IWYU pragma: export
#include "stator/integer_sign.h"   // IWYU pragma: export
#include "stator/member_sign.h"    // IWYU pragma: export
#include "stator/name_sign.h"      // IWYU pragma: export
#include "stator/record_sign.h"    // IWYU pragma: export
#include "stator/variable_sign.h"  // IWYU pragma: export
#include "stator/vector_sign.h"    // IWYU pragma: export

#include "stator/constant_stmt.h"  // IWYU pragma: export
#include "stator/type_stmt.h"      // IWYU pragma: export

#include "stator/boolean_type.h"   // IWYU pragma: export
#include "stator/integer_type.h"   // IWYU pragma: export
#include "stator/variable_type.h"  // IWYU pragma: export
#include "stator/vector_type.h"    // IWYU pragma: export

#include <stddef.h>

/// @internal Used to emit each abstract branch in a cast
#define MU_CAST_EMIT(l, upper, t, ...) || _kind == MU_##upper##__VA_ARGS__

/// @internal Used to emit each branch in mu_expr_cast()
#define MU_EXPR_CAST_EMIT(lower, upper, t, ...) \
  , const mu_##lower##_expr_t *: _kind == MU_##upper##_EXPR##__VA_ARGS__

/// @internal Used to emit each branch in mu_sign_cast()
#define MU_SIGN_CAST_EMIT(lower, upper, t, ...) \
  , const mu_##lower##_sign_t *: _kind == MU_##upper##_SIGN##__VA_ARGS__

/// @internal Used to emit each branch in mu_stmt_cast()
#define MU_STMT_CAST_EMIT(lower, upper, t, ...) \
  , const mu_##lower##_stmt_t *: _kind == MU_##upper##_STMT##__VA_ARGS__

/// @internal Used to emit each branch in mu_type_cast()
#define MU_TYPE_CAST_EMIT(lower, upper, t, ...) \
  , const mu_##lower##_type_t *: _kind == MU_##upper##_TYPE##__VA_ARGS__

#define mu_stator_cast(abstract, concrete) ({ \
    const mu_stator_t *_abstract = (abstract); \
    typeof(concrete) _concrete; \
    \
    mu_stator_kind_t _kind = _abstract->kind; \
    _Bool _castable = _Generic(_concrete, \
      const mu_name_t *: _kind == MU_NAME_STATOR, \
      const mu_node_t *: 0 MU_EACH_NODE_KIND(MU_CAST_EMIT, _STATOR), \
      const mu_expr_t *: 0 MU_EACH_EXPR_KIND(MU_CAST_EMIT, _EXPR_STATOR), \
      const mu_sign_t *: 0 MU_EACH_SIGN_KIND(MU_CAST_EMIT, _SIGN_STATOR), \
      const mu_stmt_t *: 0 MU_EACH_STMT_KIND(MU_CAST_EMIT, _STMT_STATOR), \
      const mu_type_t *: 0 MU_EACH_TYPE_KIND(MU_CAST_EMIT, _TYPE_STATOR) \
      MU_EACH_EXPR_KIND(MU_EXPR_CAST_EMIT, _STATOR) \
      MU_EACH_SIGN_KIND(MU_SIGN_CAST_EMIT, _STATOR) \
      MU_EACH_STMT_KIND(MU_STMT_CAST_EMIT, _STATOR) \
      MU_EACH_TYPE_KIND(MU_TYPE_CAST_EMIT, _STATOR)); \
    _castable ? (typeof(_concrete)) _abstract : NULL; \
  })

/**
 * @brief Downcast the @a abstract node to the <tt>typeof(concrete)</tt>
 *
 * @a abstract should have type <tt>const mu_node_t *</tt>. @a concrete should
 * be, or have, the type of a pointer to a const qualified concrete node, or:
 *
 * - <tt>const mu_expr_t *</tt>
 * - <tt>const mu_sign_t *</tt>
 * - <tt>const mu_stmt_t *</tt>
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
 *   - or a const qualified pointer to a concrete node
 */
#define mu_node_cast(abstract, concrete) ({ \
    const mu_node_t *_abstract = (abstract); \
    typeof(concrete) _concrete; \
    \
    mu_node_kind_t _kind = _abstract->kind; \
    _Bool _castable = _Generic(_concrete, \
      const mu_expr_t *: 0 MU_EACH_EXPR_KIND(MU_CAST_EMIT, _EXPR_NODE), \
      const mu_sign_t *: 0 MU_EACH_SIGN_KIND(MU_CAST_EMIT, _SIGN_NODE), \
      const mu_stmt_t *: 0 MU_EACH_STMT_KIND(MU_CAST_EMIT, _STMT_NODE) \
      MU_EACH_EXPR_KIND(MU_EXPR_CAST_EMIT, _NODE) \
      MU_EACH_SIGN_KIND(MU_SIGN_CAST_EMIT, _NODE) \
      MU_EACH_STMT_KIND(MU_STMT_CAST_EMIT, _NODE)); \
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
#define mu_expr_cast(abstract, concrete) ({ \
    const mu_expr_t *_abstract = (abstract); \
    typeof(concrete) _concrete; \
    \
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
#define mu_sign_cast(abstract, concrete) ({ \
    const mu_sign_t *_abstract = (abstract); \
    typeof(concrete) _concrete; \
    \
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
#define mu_stmt_cast(abstract, concrete) ({ \
    const mu_stmt_t *_abstract = (abstract); \
    typeof(concrete) _concrete; \
    \
    mu_stmt_kind_t _kind = _abstract->kind; \
    int _castable = _Generic(_concrete MU_EACH_STMT_KIND(MU_STMT_CAST_EMIT)); \
    _castable ? (typeof(_concrete)) _abstract : NULL; \
  })

/**
 * @brief Downcast the @a abstract type to the <tt>typeof(concrete)</tt>
 *
 * @a abstract should have type <tt>const mu_type_t *</tt>. @a concrete should
 * be, or have, the type of a pointer to a const qualified concrete type. Then
 * if @a abstract is an instance of that type, it will be cast to that type and
 * returned. Otherwise, this will return @c NULL.
 *
 * @par Example:
 * @code{.c}
 *   mu_type_t *abstract_type = ...;
 *
 *   mu_vector_type_t *type;
 *   if ((type = mu_type_cast(abstract_type, type)) == NULL)
 *     return ...;
 * @endcode
 *
 * The behavior is undefined if:
 * - @a abstract is @c NULL
 * - @a abstract doesn't have type <tt>const mu_type_t *</tt>
 * - @a concrete isn't, or doesn't have, the type of a const qualified pointer
 *   to a concrete type
 */
#define mu_type_cast(abstract, concrete) ({ \
    const mu_type_t *_abstract = (abstract); \
    typeof(concrete) _concrete; \
    \
    mu_type_kind_t _kind = _abstract->kind; \
    int _castable = _Generic(_concrete MU_EACH_TYPE_KIND(MU_TYPE_CAST_EMIT)); \
    _castable ? (typeof(_concrete)) _abstract : NULL; \
  })

/// Emit debugging information on the abstract @a node to the debug stream
void mu_node_debug(const mu_node_t *node) __attribute__((nonnull));

/// Emit debugging information on the abstract @a expr to the debug stream
void mu_expr_debug(const mu_expr_t *expr) __attribute__((nonnull));

/// Emit debugging information on the abstract @a sign to the debug stream
void mu_sign_debug(const mu_sign_t *sign) __attribute__((nonnull));

/// Emit debugging information on the abstract @a stmt to the debug stream
void mu_stmt_debug(const mu_stmt_t *stmt) __attribute__((nonnull));

/// Emit debugging information on the abstract @a type to the debug stream
void mu_type_debug(const mu_type_t *type) __attribute__((nonnull));

#endif /* MU_STATOR_H */
