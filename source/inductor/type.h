#ifndef MU_INDUCTOR_TYPE_I
#define MU_INDUCTOR_TYPE_I

#include "core.h"

#include <assert.h>
#include <stddef.h>

typedef struct induce_t induce_t;

typedef enum {
  MU_CORE_TYPE, MU_SCHEME_TYPE, MU_VARIABLE_TYPE,
} mu_type_kind_t;

/// An abstract type
typedef struct {
  mu_type_kind_t kind;
  const induce_t *induce;
  size_t id;

  // TODO
  union {
    unsigned int status;
    struct {
      _Bool access[2];
      _Bool polymorphic;
    };
  };
} mu_type_t;

/// The header that each concrete type must have
#define MU_TYPE_HEADER mu_type_t as_type

/// An abstract solution
typedef struct {
  enum {
    MU_CORE_SOLUTION = MU_CORE_TYPE,
    MU_SCHEME_SOLUTION = MU_SCHEME_TYPE,
    MU_JOIN_SOLUTION,
    /* MU_MEET_SOLUTION, */
  } kind;
} mu_solution_t;

/// The header that each concrete solution must have
#define MU_SOLUTION_HEADER mu_solution_t as_solution

/// An abstract static type
typedef struct { union {
  MU_TYPE_HEADER;
  MU_SOLUTION_HEADER;

  enum {
    MU_CORE_STATIC_TYPE = MU_CORE_TYPE,
    MU_SCHEME_STATIC_TYPE = MU_SCHEME_TYPE,
  } kind;
}; } mu_static_type_t;

/// The header that each concrete static type must have
#define MU_STATIC_TYPE_HEADER union { \
  MU_TYPE_HEADER; MU_SOLUTION_HEADER; \
  mu_static_type_t as_static_type; \
}

typedef struct mu_variable_type_t mu_variable_type_t;

/// A core type
typedef struct {
  MU_STATIC_TYPE_HEADER;
  const mu_core_t *core;
  const mu_type_t *argv[/* core->argc */];
} mu_core_type_t;

/// A scheme type
typedef struct {
  MU_STATIC_TYPE_HEADER;

  const mu_type_t *matter;

  /// Length of list of polymorphic variables
  size_t argc;
  const mu_type_t *argv[/* argc */];
} mu_scheme_type_t;

/// A join solution
typedef struct {
  MU_SOLUTION_HEADER;
  const induce_t *induce;
  size_t argc;
  const mu_type_t *argv[/* argc */];
} mu_join_t;

typedef struct mu_scheme_t mu_scheme_t;
struct mu_scheme_t {
  const induce_t *induce;
  mu_scheme_t *parent;

  // The lowest id that a type that's a part of this scheme will have
  size_t id;
};

/// A variable type
struct mu_variable_type_t {
  MU_TYPE_HEADER;

  const mu_solution_t *solution;

  // Debugging
  size_t number; ///< Used to generate a name

  // Polymorphism
  _Bool reduced;

  const mu_scheme_type_t *scheme;
};

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
  typeof(_abstract->kind) _kind = _abstract->kind; \
  int _castable = _Generic(_concrete, \
    const mu_static_type_t *: \
      _kind == MU_CORE_TYPE || _kind == MU_SCHEME_TYPE, \
    const mu_core_type_t *: _kind == MU_CORE_TYPE, \
    const mu_scheme_type_t *: _kind == MU_SCHEME_TYPE, \
    const mu_variable_type_t *: _kind == MU_VARIABLE_TYPE); \
  _castable ? (typeof(_concrete)) _abstract : NULL; \
})

#define mu_static_type_cast(abstract, concrete) __extension__ ({ \
  const mu_static_type_t *_abstract = (abstract); \
  typeof(concrete) _concrete; \
  typeof(_abstract->kind) _kind = _abstract->kind; \
  int _castable = _Generic(_concrete, \
    const mu_core_type_t *: _kind == MU_CORE_STATIC_TYPE, \
    const mu_scheme_type_t *: _kind == MU_SCHEME_STATIC_TYPE); \
  _castable ? (typeof(_concrete)) _abstract : NULL; \
})

#define mu_solution_cast(abstract, concrete) __extension__ ({ \
  const mu_solution_t *_abstract = (abstract); \
  typeof(concrete) _concrete; \
  typeof(_abstract->kind) _kind = _abstract->kind; \
  int _castable = _Generic(_concrete, \
    const mu_static_type_t *: \
      _kind == MU_CORE_SOLUTION || _kind == MU_SCHEME_SOLUTION, \
    const mu_core_type_t *: _kind == MU_CORE_SOLUTION, \
    const mu_scheme_type_t *: _kind == MU_SCHEME_SOLUTION, \
    const mu_join_t *: _kind == MU_JOIN_SOLUTION); \
  _castable ? (typeof(_concrete)) _abstract : NULL; \
})

const mu_core_type_t *mu_boolean_type(induce_t *induce)
  __attribute__((malloc, nonnull));

const mu_core_type_t *mu_integer_type(induce_t *induce)
  __attribute__((malloc, nonnull));

const mu_core_type_t *mu_core_type(
    induce_t *induce, const mu_core_t *core, const mu_type_t *const argv[])
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
    const mu_variable_type_t *const argv[argc])
  __attribute__((malloc, nonnull(1, 2)));

const mu_variable_type_t *mu_variable_type(induce_t *induce);

mu_scheme_t *mu_scheme(mu_scheme_t *parent)
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

void mu_solution_debug(const mu_solution_t *solution, _Bool expand)
  __attribute__((nonnull));

void type_debug(const mu_type_t *type, _Bool expand)
  __attribute__((nonnull));

void debug_variable_type_name(const mu_variable_type_t *type);

__attribute__((nonnull))
static inline const mu_solution_t *assign_solution(
    const mu_variable_type_t *variable_type, const mu_solution_t *solution) {
  return ((mu_variable_type_t *) variable_type)->solution = solution;
}

/// @internal An enumeration over each kind of type, e.g. @c _core_type_kind
enum {
  _core_type_kind = MU_CORE_TYPE,
  _variable_type_kind = MU_VARIABLE_TYPE,
  _scheme_type_kind = MU_SCHEME_TYPE,
  _join_kind = MU_JOIN_SOLUTION,
  /* _meet_solution_kind = MU_MEET_SOLUTION, */
};

#endif /* MU_INDUCTOR_TYPE_I */
