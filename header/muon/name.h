#ifndef MU_NAME_H
#define MU_NAME_H

#include "common.h"
#include "engine.h"

#include <stddef.h>

typedef struct {
  const mu_engine_t *engine;
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

/// Return the actual text of the @a name as a null terminated string
const mu_char8_t *mu_name_text(const mu_name_t *name)
  __attribute__((const, nonnull, returns_nonnull));

/// Return the length of the @a name (not counting the null terminator)
size_t mu_name_length(const mu_name_t *name)
  __attribute__((nonnull, pure));

#endif /* MU_NAME_H */
