#ifndef MU_COMMON_I
#define MU_COMMON_I

#include <muon/common.h> // IWYU pragma: export

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

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

MUON_HINT(const) static inline _Bool struct_size_overflow(
    size_t nought, size_t offset, size_t size, size_t *length) {
  if (rare(*length > (SIZE_MAX - offset) / size))
    return 1;
  *length = maximum(offset + size * *length, nought);
  return 0;
}

#define struct_size_overflow(struct, member, length) rare(struct_size_overflow( \
    sizeof(struct), \
    offsetof(struct, member), \
    sizeof((struct) {0}.member[0]), /* NOLINT(bugprone-sizeof-expression) */ \
    (length)))

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

// TODO: better documentation

MUON_HINT(unused) static _Thread_local const void *abstract_object;

/**
 * @brief Used to switch on the kind of the abstract @a object
 *
 * The @a object must be a pointer to an object with a @c kind member of
 * integer type.
 */
#define ON_ABSTRACT_OBJECT(object) ( \
  (abstract_object = (object)), ((typeof((object))) abstract_object)->tag \
)

/// Whether to emit symbol debugging information
extern _Thread_local _Bool debug_scan;

/// Literal printf specifier for a kind
#define PRIsKIND "%s%s%s"

/// Used with PRIsKIND to emit the @a text as a node kind
#define DEBUG_NODE_KIND(text) \
  mu_debug_colorize ? "\x1b[0;33m" : "", (text), mu_debug_colorize ? "\x1b[0m" : ""

/// Used with PRIsKIND to emit the @a text as a core kind
#define DEBUG_CORE_KIND(text) "", (text), ""

/// Used with PRIsKIND to emit the @a text as a coercion kind
#define DEBUG_COERCION_KIND(text) \
  mu_debug_colorize ? "\x1b[0;34m" : "", (text), mu_debug_colorize ? "\x1b[0m" : ""

/// Literal printf specifier for a name
#define PRIsNAME "%s"

/// Used with PRIsNAME to emit the text of the @a name
#define DEBUG_NAME(name) ((name)->text)

/// Like <tt>fprintf(muon_debug_stream, format, ...)</tt>
MUON_HINT(format(printf, 1, 2), nonnull(1))
static inline void debug(const char *restrict format, ...) {
  FILE *stream = muon_debug_stream != NULL ? muon_debug_stream : stderr;

  va_list variadic;
  va_start(variadic, format);
  vfprintf(stream, format, variadic);
  va_end(variadic);
}

#endif /* MU_COMMON_I */
