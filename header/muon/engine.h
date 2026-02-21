#ifndef MU_ENGINE_H
#define MU_ENGINE_H

#include "engine/common.h" // IWYU pragma: export
#include "engine/name.h"   // IWYU pragma: export
#include "engine/node.h"   // IWYU pragma: export

/**
 * @brief Return the minimum enumerator indicative of the concrete @a stator
 *
 * @a stator must be a pointer to a concrete stator.
 *
 * This is equivalent to MUON_STATOR_ENUMERATOR() if @a stator is a concrete
 * stator. Otherwise (e.g. if @a stator is a <tt>MuonNode *</tt>), then this
 * returns the
 */
#define MUON_STATOR_ENUMERATOR_MINIMUM(stator) \
  _Generic((typeof(stator)) {}, \
    MUON_EACH_STATOR_STEM(MUON_ENUMERATOR_EMIT,,, STATOR), \
      MuonNode *: MUON_MINORANT_NODE_STATOR, \
      MuonExpr *: MUON_MINORANT_EXPR_STATOR, \
      MuonSign *: MUON_MINORANT_SIGN_STATOR)

#endif /* MU_ENGINE_H */
