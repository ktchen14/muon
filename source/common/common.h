#ifndef MUON_COMMON_COMMON_I
#define MUON_COMMON_COMMON_I

#include <muon/common.h> // IWYU pragma: export

#include <stddef.h>
#include <stdint.h>

/// Indicate that @c ... will, in the common case, evaluate to 1
#define common(...) __builtin_expect((__VA_ARGS__), 1)

/// Indicate that @c ... will, in the common case, evaluate to 0
#define rare(...)   __builtin_expect((__VA_ARGS__), 0)

/// Return @a a, then move @a b into @a a
#define MOVE(a, b) __extension__ ({ \
  auto _a = &(a); auto _result = *_a; *_a = (b), _result; \
})

/// Return the minimum of @a a and @a b (as defined by the @c < operator)
#define minimum(a, b) __extension__ ({ \
  typeof((a)) _a = (a); typeof((b)) _b = (b); _a < _b ? _a : _b; \
})

/// Return the maximum of @a a and @a b (as defined by the @c > operator)
#define maximum(a, b) __extension__ ({ \
  typeof((a)) _a = (a); typeof((b)) _b = (b); _a > _b ? _a : _b; \
})

/**
 * @brief Return the size of a struct with a flexible array member
 *
 * @param nought size of the struct sans the flexible array member
 * @param offset offset of the flexible array member into the struct
 * @param member size of a single element of the flexible array
 * @param length length of the flexible array
 * @return the size of the struct with the flexible array member
 */
MUON_HINT(const) static inline size_t struct_size2(
    size_t nought, size_t offset, size_t member, size_t length) {
  return maximum(offset + member * length, nought);
}

#define struct_size2(T, member, length) struct_size2( \
  sizeof(T), offsetof(T, member), sizeof((T) {}.member[0]), (length) \
)

/**
 * @brief Assess the size of a struct with a flexible array member
 *
 * @param nought size of the struct sans the flexible array member
 * @param offset offset of the flexible array member into the struct
 * @param member size of a single element of the flexible array
 * @param size length of the flexible array as well as the result
 * @return 1 if the result will overflow a @c size_t; otherwise 0
 */
MUON_HINT(const) static inline _Bool struct_size_overflow(
    size_t nought, size_t offset, size_t member, size_t *size) {
  if (rare(*size > (SIZE_MAX - offset) / member))
    return 1;
  *size = (struct_size2)(nought, offset, member, *size);
  return 0;
}

#define struct_size_overflow(T, member, size) \
  rare(struct_size_overflow( \
    sizeof(T), offsetof(T, member), sizeof((T) {}.member[0]), (size) \
  ))

/**
 * @brief Return size to allocate to hold a struct with a flexible array member
 *
 * This will return zero if the result will overflow a @c size_t.
 *
 * @param nought size of the struct with the flexible array member omitted
 * @param offset offset of the flexible array member into the struct
 * @param size size of an element of the flexible array member
 * @param length intended length of the struct's flexible array member
 * @return the size of the struct, or zero if it will overflow a @c size_t
 */
MUON_HINT(const) static inline size_t struct_size(
    size_t nought, size_t offset, size_t size, size_t length) {
  size_t result;
  if (rare(__builtin_mul_overflow(length, size, &result)))
    return 0;
  if (rare(__builtin_add_overflow(result, offset, &result)))
    return 0;
  return maximum(result, nought);
}

/**
 * @brief Return size to allocate to hold a struct with a flexible array member
 *
 * This is like struct_size(), except that the allocation is specified through
 * the type name of the struct and the member name of the flexible array member
 * rather than through @a nought, @a member_offset, and @a member_size.
 *
 * Note that @a member doesn't have to be a direct member of the @a struct. Any
 * member name compatible with offsetof() is acceptable.
 *
 * This will return zero if the calculated size will overflow a @c size_t.
 *
 * @param struct name of the struct type
 * @param member name of the flexible array member within the @a struct
 * @param length intended length of the flexible array @a member
 * @return the size of the struct, or zero if it will overflow a @c size_t
 */
#define struct_size(struct, member, length) struct_size( \
    sizeof(struct), \
    offsetof(struct, member), \
    sizeof((struct) {0}.member[0]), /* NOLINT(bugprone-sizeof-expression) */ \
    (length))

/// Return @a offset into the @a object, or @c NULL on a @c NULL @a object
MUON_HINT(const)
static inline const void *object_member(const void *object, size_t offset) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
  return object != NULL ? ((const char *) object + offset) : NULL;
#pragma GCC diagnostic pop
}

#endif /* MUON_COMMON_COMMON_I */
