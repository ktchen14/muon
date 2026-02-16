#ifndef MUON_ENGINE_NAME_H
#define MUON_ENGINE_NAME_H

#include "common.h"

#include <stddef.h>

/**
 * @brief A symbol in Muon.
 *
 * Note that a MuonName is a constant object; the mutable equivalent is a
 * struct MuonName.
 */
typedef const struct MuonName {
  const MuonEngine *engine;

  /// The length of the name (not counting the null terminator)
  size_t length;

  char text[/* length + sizeof('\0') */];
} MuonName;

/**
 * @brief Define a name in the @a engine
 *
 * The actual @a text of the name must be a null terminated string in UTF-8
 * encoding.
 */
MuonName *muon_name(
  MuonEngine *engine, size_t length, const char text[restrict static length])
  /**/ MUON_NONNULL;

void muon_name_debug(MuonName *name) MUON_NONNULL;

#endif /* MUON_ENGINE_NAME_H */
