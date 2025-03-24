#ifndef MU_COMMON_I
#define MU_COMMON_I

#include <stddef.h>
#include <stdio.h>

/// Indicate that @c ... will, in the common case, evaluate to 1
#define common(...) __builtin_expect((__VA_ARGS__), 1)

/// Indicate that @c ... will, in the common case, evaluate to 0
#define rare(...)   __builtin_expect((__VA_ARGS__), 0)

/// Return the minimum of @a a and @a b (as defined by the @c < operator)
#define minimum(a, b) __extension__ ({ \
  typeof((a)) _a = (a); typeof((b)) _b = (b); _a < _b ? _a : _b; \
})

/// Return the maximum of @a a and @a b (as defined by the @c > operator)
#define maximum(a, b) __extension__ ({ \
  typeof((a)) _a = (a); typeof((b)) _b = (b); _a > _b ? _a : _b; \
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

// TODO: better documentation

__attribute__((unused)) static _Thread_local const void *_object;

#define JOIN(a, b) a##b

/// Join @a a and @a b with a level of indirection
#define INDIRECT_JOIN(a, b) JOIN(a, b)

/**
 * @brief Used to switch on the kind of the abstract @a object
 *
 * The @a object must be a pointer to an object with a @c kind member of
 * integer type.
 */
#define ON_ABSTRACT_OBJECT(object) ( \
  (_object = (object)), ((typeof((object))) _object)->kind \
)

/**
 * @brief Used to define a case of a specific kind
 *
 * This shouldn't be used, except inside a switch statement where
 * ON_ABSTRACT_OBJECT is used to define the switch expression.
 *
 * This will render as code suitable for use after the @c case keyword to match
 * the case of _##name##_kind.
 *
 * It will also declare a variable of type @c mu_##name##_t, with name @c name,
 * that will be set to the abstract object (specified in ON_ABSTRACT_OBJECT()).
 */
#define IS_KIND_OF(name) _##name##_kind:; \
  const mu_##name##_t *name = _object; \
  goto INDIRECT_JOIN(case_on_, __LINE__); \
  INDIRECT_JOIN(case_on_, __LINE__)

/// Whether to colorize the debug output
extern _Thread_local _Bool debug_colorize;

/// The amount of indentation to insert before each line of debug output
extern _Thread_local int debug_indent;

/// Whether to debug a type as a negative or positive type
extern _Thread_local _Bool debug_negate;

/// Stream to emit debugging output to (defaults to @c stderr)
extern FILE *debug_stream;

/// Literal printf specifier for a kind
#define PRIsKIND "%s%s%s"

/// Used with PRIsKIND to emit the @a text as a node kind
#define DEBUG_NODE_KIND(text) \
  debug_colorize ? "\x1b[0;33m" : "", (text), debug_colorize ? "\x1b[0m" : ""

/// Used with PRIsKIND to emit the @a text as a core kind
#define DEBUG_CORE_KIND(text) "", (text), ""

/// Used with PRIsKIND to emit the @a text as a coercion kind
#define DEBUG_COERCION_KIND(text) \
  debug_colorize ? "\x1b[0;34m" : "", (text), debug_colorize ? "\x1b[0m" : ""

/// Literal printf specifier for a name
#define PRIsNAME "%s"

/// Used with PRIsNAME to emit the text of the @a name
#define DEBUG_NAME(name) ((name)->text)

#define WITH_DEBUG_INDENT() \
  for (int _i = (debug_indent += 2); debug_indent == _i; debug_indent -= 2)

#define WITH_DEBUG_NEGATE() \
  for (_Bool _n = (debug_negate = !debug_negate); debug_negate == _n; debug_negate = !debug_negate)

/// Equivalent to <tt>printf(debug_stream, ...)</tt>
#define debug(...) fprintf(debug_stream, ##__VA_ARGS__)

#endif /* MU_COMMON_I */
