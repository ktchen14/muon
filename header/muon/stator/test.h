#ifndef MU_STATOR_TEST_H
#define MU_STATOR_TEST_H

#include "abstract_test.h"  // IWYU pragma: export
#include "member_test.h"    // IWYU pragma: export

#include <stddef.h>

/// @internal Used to emit each branch in mu_test_cast()
#define MU_TEST_CAST_EMIT(lower, upper, t, ...) \
  , const mu_##lower##_test_t *: _kind == MU_##upper##_TEST##__VA_ARGS__

/**
 * @brief Downcast the @a abstract test to the <tt>typeof(concrete)</tt>
 *
 * @a abstract should have type <tt>const mu_test_t *</tt>. @a concrete should
 * be, or have, the type of a pointer to a const qualified concrete test. Then
 * if @a abstract is an instance of that type, it will be cast to that type and
 * returned. Otherwise, this will return @c NULL.
 *
 * @par Example:
 * @code{.c}
 *   mu_test_t *abstract_test = ...;
 *
 *   mu_vector_test_t *test;
 *   if ((test = mu_test_cast(abstract_test, test)) == NULL)
 *     return ...;
 * @endcode
 *
 * The behavior is undefined if:
 * - @a abstract is @c NULL
 * - @a abstract doesn't have type <tt>const mu_test_t *</tt>
 * - @a concrete isn't, or doesn't have, the type of a const qualified pointer
 *   to a concrete test
 */
#define mu_test_cast(abstract, concrete) ({ \
    const mu_test_t *_abstract = (abstract); \
    typeof(concrete) _concrete; \
    \
    mu_test_kind_t _kind = _abstract->kind; \
    int _castable = _Generic(_concrete MU_EACH_TEST_KIND(MU_TEST_CAST_EMIT)); \
    _castable ? (typeof(_concrete)) _abstract : NULL; \
  })

/// Emit debugging information on the abstract @a test to the debug stream
void mu_test_debug(const mu_test_t *test) __attribute__((nonnull));

#endif /* MU_STATOR_TEST_H */
