#ifndef MU_INDUCTOR_TYPE_I
#define MU_INDUCTOR_TYPE_I

#include "core.h"
#include "../stator/name.h"

#include <assert.h>

/// Expands to emit(lower, upper, title, ...) for each kind of type
#define MU_EACH_TYPE_KIND(emit, ...) \
  emit(simple, SIMPLE, Simple, ##__VA_ARGS__) \
  emit(record, RECORD, Record, ##__VA_ARGS__) \
  emit(variable, VARIABLE, Variable, ##__VA_ARGS__) \
  emit(scheme, SCHEME, Scheme, ##__VA_ARGS__) \
  emit(join, JOIN, Join, ##__VA_ARGS__)

/// An enumeration over each kind of type
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_TYPE,
  MU_EACH_TYPE_KIND(MU_EMIT)
#undef MU_EMIT
} mu_type_kind_t;

typedef struct induce_t induce_t;

/// An abstract type
typedef struct {
  mu_type_kind_t kind;
  induce_t *induce;
  size_t id;
} mu_type_t;

/// The header that each concrete type must have
#define MU_TYPE_HEADER mu_type_t as_type

/// A simple type
typedef struct {
  MU_TYPE_HEADER;

  const mu_core_t *core;
  const mu_type_t *argv[/* core->argc */];
} mu_simple_type_t;

/// A record type member
typedef struct {
  const mu_name_t *name;
  const mu_type_t *type;
} mu_type_member_t;

/// A record type
typedef struct {
  MU_TYPE_HEADER;
  size_t argc;
  mu_type_member_t argv[/* argc */];
} mu_record_type_t;

typedef struct mu_scheme_type_t mu_scheme_type_t;
typedef struct mu_variable_type_t mu_variable_type_t;

/// A variable type
struct mu_variable_type_t {
  MU_TYPE_HEADER;

  mu_variable_type_t *scheme_next;
  const mu_variable_type_t *debug_next;

  size_t number; ///< Used to generate a name

  size_t rank;
  const mu_scheme_type_t *polymorphic_to;

  _Bool positively_reachable;
  _Bool negatively_reachable;

  const mu_variable_type_t *positively_entered_from;
  const mu_variable_type_t *negatively_entered_from;

  const mu_type_t *assignment;
};

/// A scheme type
struct mu_scheme_type_t {
  MU_TYPE_HEADER;

  const mu_type_t *matter;

  /// Length of list of polymorphic variables
  size_t argc;
  const mu_variable_type_t *argv[/* argc */];
};

// A join type (⊔ or ⊥)
typedef struct {
  MU_TYPE_HEADER;
  size_t argc;
  const mu_type_t *argv[/* argc */];
} mu_join_type_t;

/// @internal Used to emit each branch in mu_type_cast()
#define MU_TYPE_CAST_EMIT(lower, upper, t) \
  , const mu_##lower##_type_t *: _kind == MU_##upper##_TYPE

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
 *   mu_simple_type_t *type;
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

typedef struct {
  const mu_variable_type_t *next;
} type_link_t;

const mu_simple_type_t *mu_boolean_type(induce_t *induce)
  __attribute__((malloc, nonnull));

const mu_simple_type_t *mu_integer_type(induce_t *induce)
  __attribute__((malloc, nonnull));

const mu_join_type_t *join_type(
    induce_t *induce, size_t argc, const mu_type_t *argv[/* argc */])
  __attribute__((malloc, nonnull));

const mu_simple_type_t *mu_lambda_type(
    induce_t *induce, const mu_type_t *argument, const mu_type_t *output)
  __attribute__((malloc, nonnull));

const mu_record_type_t *mu_record_type(
    induce_t *induce, size_t argc, const mu_type_member_t argv[argc])
  __attribute__((malloc, nonnull(1)));

const mu_simple_type_t *mu_vector_type(
    induce_t *induce, const mu_type_t *matter)
  __attribute__((malloc, nonnull));

const mu_scheme_type_t *mu_scheme_type(
    induce_t *induce,
    const mu_type_t *matter,
    size_t argc,
    const mu_variable_type_t *argv[argc])
  __attribute__((malloc, nonnull(1, 2)));

mu_record_type_t *record_type_allocate(induce_t *induce, size_t argc)
  __attribute__((malloc, nonnull));

const mu_record_type_t *record_type_activate(mu_record_type_t *type)
  __attribute__((nonnull));

mu_scheme_type_t *scheme_type_allocate(induce_t *induce, size_t argc)
  __attribute__((malloc, nonnull));

const mu_scheme_type_t *scheme_type_activate(
    mu_scheme_type_t *type, const mu_type_t *matter)
  __attribute__((nonnull));

mu_join_type_t *join_type_allocate(induce_t *induce, size_t argc)
  __attribute__((malloc, nonnull));

const mu_join_type_t *join_type_activate(mu_join_type_t *type)
  __attribute__((nonnull));

void debug_type(const mu_type_t *type);
void debug_variable_type_name(const mu_variable_type_t *type);
void debug_just_type(const mu_type_t *type);

/// Compare the type member @a a to the type member @a b
__attribute__((nonnull, pure))
static inline int type_member_cmp(const void *a, const void *b) {
  const mu_type_member_t *ra = a, *rb = b;
  assert(ra->name != NULL && rb->name != NULL);
  return name_cmp(ra->name, rb->name);
}

static inline _Bool is_significant(const mu_variable_type_t *type) {
  return 1;
  return type->positively_entered_from == type && type->negatively_entered_from == type
    || type->polymorphic_to != NULL;
}

#endif /* MU_INDUCTOR_TYPE_I */
