#ifndef MUON_ENGINE_STATOR_H
#define MUON_ENGINE_STATOR_H

#include "common.h"

#include <limits.h>
#include <stddef.h>

/// Expands to emit(Title, lower, UPPER, ...) for each concrete subtype of
/// MuonCore
#define MUON_EACH_CORE(emit, ...) \
  emit(BooleanCore, boolean_core, BOOLEAN_CORE __VA_OPT__(,) __VA_ARGS__) \
  emit(CustomCore, custom_core, CUSTOM_CORE __VA_OPT__(,) __VA_ARGS__) \
  emit(IntegerCore, integer_core, INTEGER_CORE __VA_OPT__(,) __VA_ARGS__) \
  emit(LambdaCore, lambda_core, LAMBDA_CORE __VA_OPT__(,) __VA_ARGS__) \
  emit(RecordCore, record_core, RECORD_CORE __VA_OPT__(,) __VA_ARGS__) \
  emit(VectorCore, vector_core, VECTOR_CORE __VA_OPT__(,) __VA_ARGS__)

/// Expands to emit(Title, lower, UPPER, ...) for each concrete subtype of
/// MuonType
#define MUON_EACH_TYPE(emit, ...) \
  emit(CoreType, core_type, CORE_TYPE __VA_OPT__(,) __VA_ARGS__) \
  emit(JoinType, join_type, JOIN_TYPE __VA_OPT__(,) __VA_ARGS__) \
  emit(MeetType, meet_type, MEET_TYPE __VA_OPT__(,) __VA_ARGS__) \
  emit(SchemeType, scheme_type, SCHEME_TYPE __VA_OPT__(,) __VA_ARGS__) \
  emit(VariableType, variable_type, VARIABLE_TYPE __VA_OPT__(,) __VA_ARGS__)

/// Expands to emit(Title, lower, UPPER, ...) for each concrete subtype of
/// MuonStator
#define MUON_EACH_STATOR(emit, ...) \
  MUON_EACH_CORE(emit __VA_OPT__(,) __VA_ARGS__) \
  MUON_EACH_TYPE(emit __VA_OPT__(,) __VA_ARGS__) \
  emit(Name, name, NAME __VA_OPT__(,) __VA_ARGS__)

/// An enumeration over each concrete subtype of MuonStator
typedef enum {
#define MUON_EMIT(T, l, UPPER) MUON_##UPPER##_STATOR,
  MUON_EACH_STATOR(MUON_EMIT)
#undef MUON_EMIT
} MuonStatorTag;

/**
 * @brief An abstract stator
 *
 * Note that a MuonStator is a constant object; the mutable equivalent is a
 * <tt>struct MuonStator</tt>.
 */
typedef const struct MuonStator {
  MuonStatorTag tag : 8;
  MuonHash hash : sizeof(MuonHash) * CHAR_BIT - 8;
} MuonStator;

/// The header that each MuonStator subtype must have
#define MUON_STATOR_HEADER struct MuonStator as_stator

/// @internal Used to emit each branch in MUON_STATOR_TAG()
#define MUON_STATOR_TAG_EMIT(Title, l, UPPER) \
  , Muon##Title *: MUON_##UPPER##_STATOR

/// Return the enumerator indicative of the concrete @a stator
#define MUON_STATOR_TAG(stator) _Generic( \
  (stator) {} MUON_EACH_TYPE(MUON_STATOR_TAG_EMIT), MuonName *: MUON_NAME_STATOR)

/// @internal Used to decide the cast result in muon_stator_cast()
MUON_HINT(nonnull, pure)
static inline MuonStator *muon_stator_cast(
    MuonStator *stator, MuonStatorTag tag) {
  return stator->tag == tag ? stator : NULL;
}

/**
 * @brief Downcast the @a abstract stator to the <tt>typeof(concrete)</tt>
 */
#define muon_stator_cast(stator, concrete) ( \
  (typeof(concrete)) muon_stator_cast((stator), MUON_STATOR_TAG(typeof(concrete))) \
)

#endif /* MUON_ENGINE_STATOR_H */
