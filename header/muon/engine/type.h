#ifndef MUON_ENGINE_TYPE_H
#define MUON_ENGINE_TYPE_H

#include "common.h"
#include "core.h"
#include "stator.h"

#include <stddef.h>

/// An enumeration over each concrete subtype of MuonType, e.g.
/// @c MUON_CORE_TYPE
typedef enum {
#define MUON_EMIT(T, l, UPPER) MUON_##UPPER = MUON_##UPPER##_STATOR,
  MUON_EACH_TYPE(MUON_EMIT)
#undef MUON_EMIT

#define MUON_EMIT(T, l, UPPER) MUON_##UPPER##_STATOR,
  /// Equivalent to the minimum enumerator in MuonTypeTag
  MUON_MINORANT_TYPE = MUON_INDIRECT(MUON_TAKE, MUON_EACH_TYPE(MUON_EMIT)),
#undef MUON_EMIT
} MuonTypeTag;

enum {
#define MUON_EMIT(...) + 1
  /// Number of distinct concrete subtypes of MuonType
  MUON_TYPE_NUMBER = MUON_EACH_TYPE(MUON_EMIT),
#undef MUON_EMIT
};

typedef const struct MuonSchemeType MuonSchemeType;

/**
 * @brief An abstract type
 *
 * Note that a MuonType is a constant object; the mutable equivalent is a
 * <tt>struct MuonType</tt>.
 */
typedef const struct MuonType {
  MuonTypeTag tag;
  const MuonEngine *engine;
  size_t id;
  MuonSchemeType *scheme;
} MuonType;

/// The header that each MuonType subtype must have
#define MUON_TYPE_HEADER struct MuonType as_type

typedef const struct MuonCoreType {
  MUON_TYPE_HEADER;
  MuonCore *core;
  MuonType *argv[/* core->argc */];
} MuonCoreType;

/// A join type, e.g. α ⊔ β, or ⊥
typedef const struct MuonJoinType {
  MUON_TYPE_HEADER;
  size_t argc;
  MuonType *argv[] MUON_HINT(counted_by(argc));
} MuonJoinType;

/// A meet type, e.g. α ⊓ β, or ⊤
typedef const struct MuonMeetType {
  MUON_TYPE_HEADER;
  size_t argc;
  MuonType *argv[] MUON_HINT(counted_by(argc));
} MuonMeetType;

typedef const struct MuonSchemeType {
  MUON_TYPE_HEADER;
  MuonType *matter;
} MuonSchemeType;

typedef const struct MuonVariableType {
  MUON_TYPE_HEADER;
} MuonVariableType;

/// @internal Used to emit each branch in MUON_TYPE_TAG()
#define MUON_TYPE_TAG_EMIT(Title, l, UPPER) , Muon##Title *: MUON_##UPPER

/// Return the enumerator indicative of the concrete @a type
#define MUON_TYPE_TAG(type) _Generic( \
  (type) {} MUON_EACH_TYPE(MUON_TYPE_TAG_EMIT))

/// @internal Used to decide the cast result in muon_type_cast()
MUON_HINT(nonnull, pure)
static inline MuonType *muon_type_cast(MuonType *type, MuonTypeTag tag) {
  return type->tag == tag ? type : NULL;
}

/**
 * @brief Downcast the @a abstract type to the <tt>typeof(concrete)</tt>
 */
#define muon_type_cast(type, concrete) ( \
  (typeof(concrete)) muon_type_cast((type), MUON_TYPE_TAG(typeof(concrete))) \
)

MuonCoreType *muon_core_type(
    MuonEngine *engine, MuonCore *core, MuonType *const argv[/* core->argc */])
  MUON_HINT_SUFFIX(malloc, nonnull(1, 2));

/// Create a boolean type in the @a engine
MuonCoreType *muon_boolean_type(MuonEngine *engine)
  MUON_HINT_SUFFIX(malloc, nonnull);

/// Create an integer type in the @a engine
MuonCoreType *muon_integer_type(MuonEngine *engine)
  MUON_HINT_SUFFIX(malloc, nonnull);

/// Create a lambda type in the @a engine
MuonCoreType *muon_lambda_type(
    MuonEngine *engine, MuonType *argument, MuonType *output)
  MUON_HINT_SUFFIX(malloc, nonnull);

/// Create a vector type in the @a engine
MuonCoreType *muon_vector_type(MuonEngine *engine, MuonType *matter)
  MUON_HINT_SUFFIX(malloc, nonnull);

/// Create a join type in the @a engine
MuonJoinType *muon_join_type(
    MuonEngine *engine, size_t argc, MuonType *const argv[/* argc */])
  MUON_HINT_SUFFIX(malloc, nonnull(1));

/// Create a meet type in the @a engine
MuonMeetType *muon_meet_type(
    MuonEngine *engine, size_t argc, MuonType *const argv[/* argc */])
  MUON_HINT_SUFFIX(malloc, nonnull(1));

MuonSchemeType *muon_scheme_type(MuonEngine *engine, MuonType *matter)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonVariableType *muon_variable_type(MuonEngine *engine)
  MUON_HINT_SUFFIX(malloc, nonnull);

/// Optional arguments to muon_type_debug()
struct MuonTypeDebugArgs {
  _Bool id; ///< Whether to emit the id of each type

  /// How strongly this position binds to the parent operator; print parentheses
  /// if this expression is weaker than strength.
  unsigned int strength;
};

/// Emit debugging information on the abstract @a type to the debug stream
void muon_type_debug(MuonType *type, struct MuonTypeDebugArgs args)
  MUON_HINT_SUFFIX(nonnull);

/// Emit debugging information on the abstract @a type to the debug stream
#define muon_type_debug(type, ...) \
  muon_type_debug((type), (struct MuonTypeDebugArgs) {__VA_ARGS__})

#endif /* MUON_ENGINE_TYPE_H */
