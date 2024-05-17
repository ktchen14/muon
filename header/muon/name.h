#ifndef MU_NAME_H
#define MU_NAME_H

#include "common.h"
#include "engine.h"

#include <stddef.h>

typedef struct {
  const mu_engine_t *engine;

  /// The length of the name (not counting the null terminator)
  size_t length;

  mu_char8_t text[/* length */];
} mu_name_t;

/**
 * @brief Define a name in the @a engine
 *
 * The actual @a text of the name must be a null terminated string in UTF-8
 * encoding.
 */
const mu_name_t *mu_name(
    mu_engine_t *engine, const mu_char8_t *restrict text, size_t length)
  __attribute__((nonnull));

void mu_name_debug(const mu_name_t *name)
  __attribute__((nonnull));

#endif /* MU_NAME_H */
