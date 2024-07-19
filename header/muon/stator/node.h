#ifndef MU_STATOR_NODE_H
#define MU_STATOR_NODE_H

#include "abstract_node.h"  // IWYU pragma: export

#include "access_expr.h"    // IWYU pragma: export
#include "boolean_expr.h"   // IWYU pragma: export
#include "integer_expr.h"   // IWYU pragma: export
#include "invoke_expr.h"    // IWYU pragma: export
#include "lambda_expr.h"    // IWYU pragma: export
#include "name_expr.h"      // IWYU pragma: export
#include "record_expr.h"    // IWYU pragma: export
#include "sequence_expr.h"  // IWYU pragma: export
#include "vector_expr.h"    // IWYU pragma: export
#include "zero_expr.h"      // IWYU pragma: export

#include "boolean_sign.h"   // IWYU pragma: export
#include "integer_sign.h"   // IWYU pragma: export
#include "name_sign.h"      // IWYU pragma: export
#include "record_sign.h"    // IWYU pragma: export
#include "variable_sign.h"  // IWYU pragma: export
#include "vector_sign.h"    // IWYU pragma: export

#include "define_stmt.h"    // IWYU pragma: export
#include "type_stmt.h"      // IWYU pragma: export

#include "variable_view.h"  // IWYU pragma: export

#include <stddef.h>

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
#define mu_node_cast(abstract, concrete) ({ \
    const mu_node_t *_abstract = (abstract); \
    typeof(concrete) _concrete; \
    \
    mu_node_kind_t _kind = _abstract->kind; \
    _Bool _castable = _Generic(_concrete, \
      const mu_expr_t *: 0 MU_EACH_EXPR_KIND(MU_CAST_EMIT, _EXPR_NODE), \
      const mu_sign_t *: 0 MU_EACH_SIGN_KIND(MU_CAST_EMIT, _SIGN_NODE), \
      const mu_stmt_t *: 0 MU_EACH_STMT_KIND(MU_CAST_EMIT, _STMT_NODE), \
      const mu_view_t *: 0 MU_EACH_VIEW_KIND(MU_CAST_EMIT, _VIEW_NODE) \
      MU_EACH_EXPR_KIND(MU_EXPR_CAST_EMIT, _NODE) \
      MU_EACH_SIGN_KIND(MU_SIGN_CAST_EMIT, _NODE) \
      MU_EACH_STMT_KIND(MU_STMT_CAST_EMIT, _NODE) \
      MU_EACH_VIEW_KIND(MU_VIEW_CAST_EMIT, _NODE)); \
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
#define mu_view_cast(abstract, concrete) ({ \
    const mu_view_t *_abstract = (abstract); \
    typeof(concrete) _concrete; \
    \
    mu_view_kind_t _kind = _abstract->kind; \
    int _castable = _Generic(_concrete MU_EACH_STMT_KIND(MU_STMT_CAST_EMIT)); \
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

/// Emit debugging information on the abstract @a view to the debug stream
void mu_view_debug(const mu_view_t *view) __attribute__((nonnull));

#endif /* MU_STATOR_NODE_H */
