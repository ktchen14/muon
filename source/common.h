#ifndef MU_COMMON_I
#define MU_COMMON_I

#include <muon/common.h>

#include <stddef.h>

typedef mu_char8_t char8_t;

#define common(...) __builtin_expect((__VA_ARGS__), 1)
#define rare(...)   __builtin_expect((__VA_ARGS__), 0)

/// Return the minimum of @a a and @a b (as defined by the @c < operator)
#define minimum(a, b) ({ \
    typeof((a)) _a = (a); typeof((b)) _b = (b); _a < b ? _a : _b; \
  })

/// Return the maximum of @a a and @a b (as defined by the @c > operator)
#define maximum(a, b) ({ \
    typeof((a)) _a = (a); typeof((b)) _b = (b); _a > b ? _a : _b; \
  })

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
__attribute__((const))
static inline size_t struct_size(
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

/**
 * @brief Return the size of a struct with a flexible array member
 *
 * This is like struct_size(), except that it can't overflow.
 *
 * @param nought size of the struct with the flexible array member omitted
 * @param offset offset of the flexible array member into the struct
 * @param size size of an element of the flexible array member
 * @param length length of the struct's flexible array member
 * @return the size of the struct
 */
__attribute__((const))
static inline size_t extant_size(
    size_t nought, size_t offset, size_t size, size_t length) {
  return maximum(offset + size * length, nought);
}

#define extant_size(struct, member, length) extant_size( \
    sizeof(struct), \
    offsetof(struct, member), \
    sizeof((struct) {0}.member[0]), /* NOLINT(bugprone-sizeof-expression) */ \
    (length))

#endif /* MU_COMMON_I */
