#ifndef MU_ENGINE_H
#define MU_ENGINE_H

#include "engine/common.h"  // IWYU pragma: export
#include "engine/name.h"    // IWYU pragma: export
#include "engine/node.h"    // IWYU pragma: export

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

/**
 * @brief Return the maximum enumerator indicative of the concrete @a stator
 *
 * @a stator must be a pointer to a concrete stator.
 *
 * This is equivalent to MUON_NODE_ENUMERATOR() if @a node is the type of a
 * concrete node. However, if @a node is <tt>MuonExpr *</tt>, then this returns
 * MUON_MAJORANT_EXPR.
 */
#define MUON_NODE_ENUMERATOR_MAXIMUM(node) \
  _Generic((node) MUON_EACH_NODE_STEM(MUON_ENUMERATOR_EMIT,,, NODE), \
    MuonExpr *: MUON_MAJORANT_EXPR, \
    MuonSign *: MUON_MAJORANT_SIGN)

/// Return the enumerator indicative of the concrete @a node
#define MUON_NODE_ENUMERATOR(node) \
  _Generic((node) MUON_EACH_NODE_STEM(MUON_ENUMERATOR_EMIT,,, NODE))

#endif /* MU_ENGINE_H */
