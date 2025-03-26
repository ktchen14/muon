#ifndef MU_INDUCTOR_TYPE_I
#define MU_INDUCTOR_TYPE_I

#include "core.h"

#include <assert.h>
#include <stddef.h>

/// Expands to emit(lower, upper, title, ...) for each kind of type
#define MU_EACH_TYPE_KIND(emit, ...) \
  emit(core, CORE, Core, ##__VA_ARGS__) \
  emit(variable, VARIABLE, Variable, ##__VA_ARGS__) \
  emit(scheme, SCHEME, Scheme, ##__VA_ARGS__)

/// Expands to emit(lower, upper, title, ...) for each kind of solution
#define MU_EACH_SOLUTION_KIND(emit, ...) \
  MU_EACH_TYPE_KIND(emit) \
  emit(join, JOIN, Join, ##__VA_ARGS__)

/// An enumeration over each kind of solution, i.e. @c MU_CORE_SOLUTION
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_SOLUTION,
  MU_EACH_TYPE_KIND(MU_EMIT)
#undef MU_EMIT
} mu_solution_kind_t;

/// An enumeration over each kind of type, i.e. @c MU_CORE_TYPE
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_TYPE = MU_##upper##_SOLUTION,
  MU_EACH_TYPE_KIND(MU_EMIT)
#undef MU_EMIT
} mu_type_kind_t;

typedef struct induce_t induce_t;

/// An abstract solution
typedef struct {
  mu_solution_kind_t kind;
} mu_solution_t;

/// The header that each concrete solution must have
#define MU_SOLUTION_HEADER mu_solution_t as_solution

/// An abstract type
typedef struct {
  union { MU_SOLUTION_HEADER; mu_type_kind_t kind; };
  induce_t *induce;
  size_t id;
} mu_type_t;

/// The header that each concrete type must have
#define MU_TYPE_HEADER union { mu_type_t as_type; mu_solution_t as_solution; }

/// A core type
typedef struct {
  MU_TYPE_HEADER;
  const mu_core_t *core;
  const mu_type_t *argv[/* core->argc */];
} mu_core_type_t;

typedef struct mu_scheme_type_t mu_scheme_type_t;
typedef struct open_scheme_t open_scheme_t;

/// A join solution
typedef struct {
  MU_SOLUTION_HEADER;
  const induce_t *induce;
  size_t argc;
  const mu_type_t *argv[/* argc */];
} mu_join_t;

/// A variable type
typedef struct mu_variable_type_t mu_variable_type_t;
struct mu_variable_type_t {
  MU_TYPE_HEADER;

  const mu_solution_t *solution;
  const mu_join_t *join;

  // Debugging
  size_t number; ///< Used to generate a name

  // Polymorphism
  mu_variable_type_t *scheme_next;

  size_t rank;
  const mu_scheme_type_t *polymorphic_to;

  _Bool positively_reachable;
  _Bool negatively_reachable;
};

/// A scheme type
struct mu_scheme_type_t {
  MU_TYPE_HEADER;

  const mu_type_t *matter;

  /// Length of list of polymorphic variables
  size_t argc;
  const mu_variable_type_t *argv[/* argc */];
};

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
 *   mu_core_type_t *type;
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
#define mu_type_cast(abstract, concrete) __extension__ ({ \
    const mu_type_t *_abstract = (abstract); \
    typeof(concrete) _concrete; \
    mu_type_kind_t _kind = _abstract->kind; \
    int _castable = _Generic(_concrete MU_EACH_TYPE_KIND(MU_TYPE_CAST_EMIT)); \
    _castable ? (typeof(_concrete)) _abstract : NULL; \
  })

typedef struct mu_node_t mu_node_t;
struct open_scheme_t {
  induce_t *induce;
  const mu_node_t *node;
  open_scheme_t *parent;
  size_t rank;
  mu_variable_type_t *link;
};

typedef struct {
  const mu_variable_type_t *next;
} type_link_t;

const mu_core_type_t *mu_boolean_type(induce_t *induce)
  __attribute__((malloc, nonnull));

const mu_core_type_t *mu_integer_type(induce_t *induce)
  __attribute__((malloc, nonnull));

const mu_core_type_t *mu_core_type(
    induce_t *induce, const mu_core_t *core, const mu_type_t *argv[])
  __attribute__((malloc, nonnull(1, 2)));

const mu_core_type_t *mu_lambda_type(
    induce_t *induce, const mu_type_t *argument, const mu_type_t *output)
  __attribute__((malloc, nonnull));

const mu_core_type_t *mu_vector_type(
    induce_t *induce, const mu_type_t *matter)
  __attribute__((malloc, nonnull));

const mu_scheme_type_t *mu_scheme_type(
    induce_t *induce,
    const mu_type_t *matter,
    size_t argc,
    const mu_variable_type_t *argv[argc])
  __attribute__((malloc, nonnull(1, 2)));

const mu_variable_type_t *mu_variable_type(induce_t *induce, open_scheme_t *scheme);

open_scheme_t *open_scheme(open_scheme_t *parent, const mu_node_t *node)
  __attribute__((malloc));

mu_core_type_t *core_type_allocate(induce_t *induce, const mu_core_t *core)
  __attribute__((malloc, nonnull));

const mu_core_type_t *core_type_activate(mu_core_type_t *type)
  __attribute__((nonnull));

mu_scheme_type_t *scheme_type_allocate(induce_t *induce, size_t argc)
  __attribute__((malloc, nonnull));

const mu_scheme_type_t *scheme_type_activate(
    mu_scheme_type_t *type, const mu_type_t *matter)
  __attribute__((nonnull));

mu_join_t *join_allocate(induce_t *induce, size_t argc)
  __attribute__((malloc, nonnull));

const mu_join_t *join_activate(mu_join_t *join)
  __attribute__((nonnull));

void type_debug(const mu_type_t *type, _Bool expand)
  __attribute__((nonnull));

void debug_variable_type_name(const mu_variable_type_t *type);
void debug_just_type(const mu_type_t *type);

/// @internal An enumeration over each kind of type, e.g. @c _core_type_kind
enum {
#define MU_EMIT(lower, u, t) _##lower##_type_kind,
  MU_EACH_TYPE_KIND(MU_EMIT)
#undef MU_EMIT
};

#endif /* MU_INDUCTOR_TYPE_I */
