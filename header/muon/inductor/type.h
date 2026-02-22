#ifndef MUON_INDUCTOR_TYPE_H
#define MUON_INDUCTOR_TYPE_H

#include "common.h"
#include "core.h"

#include <stddef.h>

/// Expands to emit(title, lower, upper, ...) for each concrete type
#define MUON_EACH_TYPE(emit, ...) \
  emit(CoreType, core_type, CORE_TYPE __VA_OPT__(,) __VA_ARGS__) \
  emit(JoinType, join_type, JOIN_TYPE __VA_OPT__(,) __VA_ARGS__) \
  emit(SchemeType, scheme_type, SCHEME_TYPE __VA_OPT__(,) __VA_ARGS__) \
  emit(VariableType, variable_type, VARIABLE_TYPE __VA_OPT__(,) __VA_ARGS__)

/// An enumeration over each concrete type, e.g. @c MUON_CORE_TYPE
typedef enum {
#define MUON_EMIT(T, l, UPPER) MUON_##UPPER,
  MUON_EACH_TYPE(MUON_EMIT)

  /// Equivalent to the minimum enumerator in MuonTypeTag
  MUON_MINORANT_TYPE = MUON_INDIRECT(MUON_TAKE, MUON_EACH_TYPE(MUON_EMIT)),
#undef MUON_EMIT
} MuonTypeTag;

enum {
#define MUON_EMIT(...) + 1
  /// Number of distinct kinds of types
  MUON_TYPE_NUMBER = MUON_EACH_TYPE(MUON_EMIT),
#undef MUON_EMIT
};

/**
 * @brief An abstract type
 *
 * Note that a MuonType is a constant object; the mutable equivalent is a
 * <tt>struct MuonType</tt>.
 */
typedef const struct MuonType {
  union {
    MuonTypeTag kind, tag;
  };
  const induce_t *induce;
  size_t id;
} MuonType;

/// The header that each MuonType subtype must have
#define MUON_TYPE_HEADER struct MuonType as_type

typedef const struct MuonCoreType {
  MUON_TYPE_HEADER;
  const MuonCore *core;
  MuonType *argv[/* core->argc */];
} MuonCoreType;

typedef const struct MuonSchemeType {
  MUON_TYPE_HEADER;

  MuonType *matter;

  /// Length of list of polymorphic variables
  size_t argc;
  MuonType *argv[/* argc */];
} MuonSchemeType;

typedef const struct MuonJoinType {
  MUON_TYPE_HEADER;
  size_t argc;
  MuonType *argv[/* argc */];
} MuonJoinType;

typedef const struct MuonVariableType {
  MUON_TYPE_HEADER;

  MuonType *solution;

  // Debugging
  size_t number; ///< Used to generate a name

  // Polymorphism
  _Bool reduced;

  MuonSchemeType *scheme;
} MuonVariableType;

/// @internal Used to emit each branch in MUON_TYPE_TAG()
#define MUON_TYPE_TAG_EMIT(Title, l, UPPER) , Muon##Title *: MUON_##UPPER

/// Return the enumerator indicative of the concrete @a type
#define MUON_TYPE_TAG(type) _Generic( \
  (type) {} MUON_EACH_TYPE(MUON_TYPE_TAG_EMIT))

/// @internal Used to decide the cast result in muon_type_cast()
MUON_HINT(nonnull)
static inline MuonType *muon_type_cast(MuonType *type, MuonTypeTag tag) {
  return type->kind == tag ? type : NULL;
}

/**
 * @brief Downcast the @a abstract type to the <tt>typeof(concrete)</tt>
 */
#define muon_type_cast(type, concrete) ( \
  (typeof(concrete)) muon_type_cast((type), MUON_TYPE_TAG(typeof(concrete))) \
)

MuonCoreType *muon_core_type(
    induce_t *inductor,
    const MuonCore *core,
    MuonType *const argv[/* core->argc */])
  MUON_HINT_SUFFIX(malloc, nonnull(1, 2));

/// Create a boolean type in the @a inductor
MuonCoreType *muon_boolean_type(induce_t *induce)
  MUON_HINT_SUFFIX(malloc, nonnull);

/// Create an integer type in the @a inductor
MuonCoreType *muon_integer_type(induce_t *induce)
  MUON_HINT_SUFFIX(malloc, nonnull);

/// Create a lambda type in the @a inductor
MuonCoreType *muon_lambda_type(
    induce_t *induce, MuonType *argument, MuonType *output)
  MUON_HINT_SUFFIX(malloc, nonnull);

/// Create a vector type in the @a inductor
MuonCoreType *muon_vector_type(induce_t *induce, MuonType *matter)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonSchemeType *muon_scheme_type(
    induce_t *induce, MuonType *matter, size_t argc, MuonType *const argv[argc])
  MUON_HINT_SUFFIX(malloc, nonnull(1, 2));

MuonVariableType *muon_variable_type(induce_t *induce)
  MUON_HINT_SUFFIX(malloc, nonnull);

#endif /* MUON_INDUCTOR_TYPE_H */
