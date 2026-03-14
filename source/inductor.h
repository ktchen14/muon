#ifndef MUON_INDUCTOR_I
#define MUON_INDUCTOR_I

#include "inductor/common.h" // IWYU pragma: export
#include "inductor/induce.h" // IWYU pragma: export
#include "inductor/reduce.h" // IWYU pragma: export

#include "detector/detect.h"
#include "engine.h"

/// Initialize the @a inductor to handle nodes and types in the @a engine
MuonInductor *muon_induce_initialize(
    MuonInductor *induce,
    MuonEngine *engine,
    const detect_t *detect,
    const MuonModule *module)
  MUON_HINT_SUFFIX(nonnull);

/// Optional arguments to inductor_debug()
struct InductorDebugArgs {
  /// Optional arguments to muon_type_debug()
  struct MuonTypeDebugArgs type;

  unsigned int hide;
};

void inductor_debug(
    const MuonInductor *inductor, struct InductorDebugArgs args);

#define inductor_debug(type, ...) \
  inductor_debug((type), (struct InductorDebugArgs) {__VA_ARGS__})

#endif /* MUON_INDUCTOR_I */
