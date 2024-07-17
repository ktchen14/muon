#ifndef MU_STATOR_NAME_I
#define MU_STATOR_NAME_I

#include <muon/stator/name.h>  // IWYU pragma: export

#include "../common.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

/// Used to traverse a name prefix chain
typedef struct {
  const mu_name_t *anterior;
} name_cursor_t;

/// Used to allocate a name and find the cursor
typedef struct {
  name_cursor_t cursor;
  mu_name_t name;
} name_header_t;

__attribute__((const, nonnull, returns_nonnull))
static inline name_cursor_t *name_cursor(const mu_name_t *name) {
  static const size_t offset = offsetof(name_header_t, name);
  name_header_t *header = (name_header_t *) ((char *) name - offset);
  return &header->cursor;
}

/// Compare name @a a to name @a b
__attribute__((nonnull, pure))
static inline int name_cmp(const mu_name_t *a, const mu_name_t *b) {
  assert(a->as_stator.engine == b->as_stator.engine);

  int result;
  if ((result = memcmp(a->text, b->text, minimum(a->length, b->length))) != 0)
    return result;
  return (a->length > b->length) - (b->length > a->length);
}

#endif /* MU_STATOR_NAME_I */
