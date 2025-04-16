#ifndef MU_INDUCTOR_TYPE_H
#define MU_INDUCTOR_TYPE_H

#include "core.h"

#include <stddef.h>

typedef struct induce_t induce_t;
typedef struct induce_t mu_inductor_t;

/// Expands to emit(lower, upper, title, ...) for each kind of type
#define MU_EACH_TYPE_KIND(emit, ...) \
  emit(core, CORE, Core, ##__VA_ARGS__) \
  emit(scheme, SCHEME, Scheme, ##__VA_ARGS__) \
  emit(variable, VARIABLE, Variable, ##__VA_ARGS__) \
  emit(join, JOIN, Join, ##__VA_ARGS__)

/// An enumeration over each kind of type, e.g. @c MU_CORE_TYPE
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_TYPE,
  MU_EACH_TYPE_KIND(MU_EMIT)
#undef MU_EMIT
} mu_type_kind_t;

/// An abstract type
typedef struct {
  mu_type_kind_t kind;
  const induce_t *induce;
  size_t id;
} mu_type_t;

/// The header that each concrete type must have
#define MU_TYPE_HEADER mu_type_t as_type

/// A core type
typedef struct {
  MU_TYPE_HEADER;
  const mu_core_t *core;
  const mu_type_t *argv[/* core->argc */];
} mu_core_type_t;

/// A scheme type
typedef struct {
  MU_TYPE_HEADER;

  const mu_type_t *matter;

  /// Length of list of polymorphic variables
  size_t argc;
  const mu_type_t *argv[/* argc */];
} mu_scheme_type_t;

/// A join type
typedef struct {
  MU_TYPE_HEADER;
  size_t argc;
  const mu_type_t *argv[/* argc */];
} mu_join_type_t;

/// A variable type
typedef struct {
  MU_TYPE_HEADER;

  const mu_type_t *solution;

  // Debugging
  size_t number; ///< Used to generate a name

  // Polymorphism
  _Bool reduced;

  const mu_scheme_type_t *scheme;
} mu_variable_type_t;

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
    const mu_core_type_t *: _kind == MU_CORE_TYPE, \
    const mu_join_type_t *: _kind == MU_JOIN_TYPE, \
    const mu_scheme_type_t *: _kind == MU_SCHEME_TYPE, \
    const mu_variable_type_t *: _kind == MU_VARIABLE_TYPE); \
  _castable ? (typeof(_concrete)) _abstract : NULL; \
})

const mu_core_type_t *mu_boolean_type(induce_t *induce)
  __attribute__((malloc, nonnull));

const mu_core_type_t *mu_integer_type(induce_t *induce)
  __attribute__((malloc, nonnull));

const mu_core_type_t *mu_lambda_type(
    induce_t *induce, const mu_type_t *argument, const mu_type_t *output)
  __attribute__((malloc, nonnull));

const mu_core_type_t *mu_vector_type(
    induce_t *induce, const mu_type_t *matter)
  __attribute__((malloc, nonnull));

const mu_core_type_t *mu_core_type(
    mu_inductor_t *inductor,
    const mu_core_t *core,
    const mu_type_t *const argv[/* core->argc */])
  __attribute__((malloc, nonnull(1, 2)));

const mu_scheme_type_t *mu_scheme_type(
    induce_t *induce,
    const mu_type_t *matter,
    size_t argc,
    const mu_type_t *const argv[argc])
  __attribute__((malloc, nonnull(1, 2)));

const mu_variable_type_t *mu_variable_type(induce_t *induce);

#endif /* MU_INDUCTOR_TYPE_H */
