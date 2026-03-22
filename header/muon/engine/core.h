#ifndef MUON_ENGINE_CORE_H
#define MUON_ENGINE_CORE_H

#include "common.h"
#include "name.h"

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

/// An enumeration over each concrete subtype of MuonCore
typedef enum {
#define MUON_EMIT(T, l, UPPER) MUON_##UPPER,
  MUON_EACH_CORE(MUON_EMIT)
#undef MUON_EMIT
} MuonCoreTag;

/**
 * @brief An abstract core
 *
 * Note that a MuonCore is a constant object; the mutable equivalent is a
 * <tt>struct MuonCore</tt>.
 */
typedef const struct MuonCore {
  MuonCoreTag tag;
  const MuonEngine *engine;
} MuonCore;

/// The header that each MuonCore subtype must have
#define MUON_CORE_HEADER struct MuonCore as_core

typedef struct {
  MuonName *name;
  size_t i;
  _Bool variance;
} MuonCoreMember;

typedef const struct MuonBooleanCore {
  MUON_CORE_HEADER;
} MuonBooleanCore;

typedef const struct MuonCustomCore {
  MUON_CORE_HEADER;
  MuonName *name;
  size_t argc;
  MuonCoreMember argv[] MUON_HINT(counted_by(argc));
} MuonCustomCore;

typedef const struct MuonIntegerCore {
  MUON_CORE_HEADER;
} MuonIntegerCore;

typedef const struct MuonLambdaCore {
  MUON_CORE_HEADER;
} MuonLambdaCore;

typedef const struct MuonRecordCore {
  MUON_CORE_HEADER;
  size_t argc;
  MuonName *argv[] MUON_HINT(counted_by(argc));
} MuonRecordCore;

typedef const struct MuonVectorCore {
  MUON_CORE_HEADER;
} MuonVectorCore;

/// @internal Used to emit each branch in MUON_CORE_TAG()
#define MUON_CORE_TAG_EMIT(Title, l, UPPER) , Muon##Title *: MUON_##UPPER

/// Return the enumerator indicative of the concrete @a core subtype
#define MUON_CORE_TAG(core) _Generic( \
  (core) {} MUON_EACH_CORE(MUON_CORE_TAG_EMIT))

/// @internal Used to decide the cast result in muon_core_cast()
MUON_HINT(nonnull, pure)
static inline MuonCore *muon_core_cast(MuonCore *core, MuonCoreTag tag) {
  return core->tag == tag ? core : NULL;
}

/**
 * @brief Downcast the @a abstract core to the <tt>typeof(concrete)</tt>
 */
#define muon_core_cast(core, concrete) ( \
  (typeof(concrete)) muon_core_cast((core), MUON_CORE_TAG(typeof(concrete))) \
)

MuonCustomCore *mu_simple_core(MuonEngine *engine, MuonName *name)
  MUON_HINT_SUFFIX(nonnull);

MuonRecordCore *muon_record_core(
    MuonEngine *engine, size_t argc, MuonName *const argv[/* argv */])
  MUON_HINT_SUFFIX(nonnull);

/// Emit debugging information on the abstract @a core to the debug stream
MUON_HINT(nonnull) void muon_core_debug(MuonCore *core);

#endif /* MUON_ENGINE_CORE_H */
