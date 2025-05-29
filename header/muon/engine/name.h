#ifndef MUON_ENGINE_NAME_H
#define MUON_ENGINE_NAME_H

#include "common.h"

#include <stddef.h>

/// Define @c mu_char8_t to have the same definition as @c char8_t in C23
typedef unsigned char mu_char8_t;

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

  mu_char8_t text[/* length + sizeof('\0') */];
} MuonName;

/**
 * @brief Define a name in the @a engine
 *
 * The actual @a text of the name must be a null terminated string in UTF-8
 * encoding.
 */
MuonName *mu_name(
    MuonEngine *engine,
    size_t length,
    const mu_char8_t text[restrict static length])
  __attribute__((nonnull));

void mu_name_debug(MuonName *name) __attribute__((nonnull));

#endif /* MUON_ENGINE_NAME_H */
