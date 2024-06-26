#ifndef MU_NAME_H
#define MU_NAME_H

#include "common.h"
#include "stator.h"

#include <stddef.h>

/// A symbol in Muon
typedef struct mu_name_t mu_name_t;
struct mu_name_t {
  MU_STATOR_HEADER;

  /// If the name has a prefix, e.g. a.b
  const mu_name_t *prefix;

  /// The length of the name (not counting the null terminator)
  size_t length;

  mu_char8_t text[/* length + sizeof('\0') */];
};

/**
 * @brief Define a name in the @a engine
 *
 * The actual @a text of the name must be a null terminated string in UTF-8
 * encoding.
 */
const mu_name_t *mu_name(
    mu_engine_t *engine,
    size_t length,
    const mu_char8_t text[restrict static length])
  __attribute__((nonnull));

void mu_name_debug(const mu_name_t *name) __attribute__((nonnull));

#endif /* MU_NAME_H */
