#ifndef MU_STATOR_TYPE_H
#define MU_STATOR_TYPE_H

#include "abstract_type.h"  // IWYU pragma: export
#include "boolean_type.h"   // IWYU pragma: export
#include "integer_type.h"   // IWYU pragma: export
#include "record_type.h"    // IWYU pragma: export
#include "variable_type.h"  // IWYU pragma: export
#include "vector_type.h"    // IWYU pragma: export

#include <stddef.h>

/// @internal Used to emit each branch in mu_type_cast()
#define MU_TYPE_CAST_EMIT(lower, upper, t, ...) \
  , const mu_##lower##_type_t *: _kind == MU_##upper##_TYPE##__VA_ARGS__

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

/// Emit debugging information on the abstract @a type to the debug stream
void mu_type_debug(const mu_type_t *type) __attribute__((nonnull));

#endif /* MU_STATOR_TYPE_H */
