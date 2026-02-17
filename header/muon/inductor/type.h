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
} MuonTypeKind;

/**
 * @brief An abstract type
 *
 * Note that a MuonType is a constant object; the mutable equivalent is a
 * struct MuonType.
 */
typedef const struct MuonType {
  MuonTypeKind kind;
  const induce_t *induce;
  size_t id;
} MuonType;

/// The header that each concrete type must have
#define MUON_TYPE_HEADER struct MuonType as_type

/// A core type
typedef const struct MuonCoreType {
  MUON_TYPE_HEADER;
  const mu_core_t *core;
  MuonType *argv[/* core->argc */];
} MuonCoreType;

/// A scheme type
typedef const struct MuonSchemeType {
  MUON_TYPE_HEADER;

  MuonType *matter;

  /// Length of list of polymorphic variables
  size_t argc;
  MuonType *argv[/* argc */];
} MuonSchemeType;

/// A join type
typedef const struct MuonJoinType {
  MUON_TYPE_HEADER;
  size_t argc;
  MuonType *argv[/* argc */];
} MuonJoinType;

/// A variable type
typedef const struct MuonVariableType {
  MUON_TYPE_HEADER;

  MuonType *solution;

  // Debugging
  size_t number;  ///< Used to generate a name

  // Polymorphism
  _Bool reduced;

  MuonSchemeType *scheme;
} MuonVariableType;

MuonCoreType *mu_core_type(
    mu_inductor_t *inductor,
    const mu_core_t *core,
    MuonType *const argv[/* core->argc */])
  MUON_MALLOC MUON_NONNULL_ARGS(1, 2);

/// Create a boolean type in the @a inductor
MuonCoreType *mu_boolean_type(induce_t *induce)
  MUON_MALLOC MUON_NONNULL;

/// Create an integer type in the @a inductor
MuonCoreType *mu_integer_type(induce_t *induce)
  MUON_MALLOC MUON_NONNULL;

/// Create a lambda type in the @a inductor
MuonCoreType *mu_lambda_type(
    induce_t *induce, MuonType *argument, MuonType *output)
  MUON_MALLOC MUON_NONNULL;

/// Create a vector type in the @a inductor
MuonCoreType *mu_vector_type(induce_t *induce, MuonType *matter)
  MUON_MALLOC MUON_NONNULL;

MuonSchemeType *mu_scheme_type(
    induce_t *induce, MuonType *matter, size_t argc, MuonType *const argv[argc])
  MUON_MALLOC MUON_NONNULL_ARGS(1, 2);

MuonVariableType *mu_variable_type(induce_t *induce)
  MUON_MALLOC MUON_NONNULL;

/// @internal Used to emit each branch in MU_TYPE_ENUMERATOR()
#define MU_TYPE_ENUMERATOR_EMIT(l, upper, title) \
  , Muon##title##Type *: MU_##upper##_TYPE \
  , struct Muon##title##Type *: MU_##upper##_TYPE

/// Return the enumerator indicative of the @a concrete type
#define MU_TYPE_ENUMERATOR(concrete) \
  _Generic((concrete) {0} MU_EACH_TYPE_KIND(MU_TYPE_ENUMERATOR_EMIT))

/**
 * @brief Downcast the @a abstract type to the <tt>typeof(concrete)</tt>
 *
 * @a abstract should have type <tt>MuonType *</tt>. @a concrete should
 * be, or have, the type of a pointer to a const qualified concrete type. Then
 * if @a abstract is an instance of that type, it will be cast to that type and
 * returned. Otherwise, this will return @c NULL.
 *
 * @par Example:
 * @code{.c}
 *   MuonType *abstract_type = ...;
 *
 *   mu_core_type_t *type;
 *   if ((type = mu_type_cast(abstract_type, type)) == NULL)
 *     return ...;
 * @endcode
 *
 * The behavior is undefined if:
 * - @a abstract is @c NULL
 * - @a abstract doesn't have type <tt>MuonType *</tt>
 * - @a concrete isn't, or doesn't have, the type of a const qualified pointer
 *   to a concrete type
 */
#define mu_type_cast(abstract, concrete) __extension__ ({ \
  MuonType *_abstract = (abstract); \
  _abstract->kind == MU_TYPE_ENUMERATOR(__typeof__(concrete)) ? \
    (__typeof__(concrete)) _abstract : NULL; \
})

#endif /* MU_INDUCTOR_TYPE_H */
