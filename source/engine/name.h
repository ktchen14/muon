#ifndef MU_ENGINE_NAME_I
#define MU_ENGINE_NAME_I

#include <muon/engine/name.h>  // IWYU pragma: export

#include "../common.h"

#include <stddef.h>
#include <string.h>

/// Compare name @a a to name @a b
__attribute__((nonnull, pure))
static inline int name_cmp(const mu_name_t *a, const mu_name_t *b) {
  int result;
  if ((result = memcmp(a->text, b->text, minimum(a->length, b->length))) != 0)
    return result;
  return (a->length > b->length) - (b->length > a->length);
}

#endif /* MU_ENGINE_NAME_I */
