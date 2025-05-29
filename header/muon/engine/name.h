#ifndef MU_ENGINE_NAME_H
#define MU_ENGINE_NAME_H

#include "common.h"

#include <stddef.h>

/// Define @c mu_char8_t to have the same definition as @c char8_t in C23
typedef unsigned char mu_char8_t;

/**
 * @brief A symbol in Muon.
 *
 * Note that a mu_name_t is a constant object; the mutable equivalent is a
 * struct mu_name_t.
 */
typedef const struct mu_name_t {
  const mu_engine_t *engine;

  /// The length of the name (not counting the null terminator)
  size_t length;

  mu_char8_t text[/* length + sizeof('\0') */];
} mu_name_t;

/**
 * @brief Define a name in the @a engine
 *
 * The actual @a text of the name must be a null terminated string in UTF-8
 * encoding.
 */
mu_name_t *mu_name(
    mu_engine_t *engine,
    size_t length,
    const mu_char8_t text[restrict static length])
  __attribute__((nonnull));

void mu_name_debug(mu_name_t *name) __attribute__((nonnull));

#endif /* MU_ENGINE_NAME_H */
