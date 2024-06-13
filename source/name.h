#ifndef MU_NAME_I
#define MU_NAME_I

#include <muon/name.h>  // IWYU pragma: export

#include "common.h"

/// Used to traverse a name prefix chain
typedef struct {
  const mu_name_t *anterior;
} name_cursor_t;

/// Used to allocate a name and find the cursor
typedef struct {
  name_cursor_t cursor;
  mu_name_t name;
} name_header_t;

static inline name_cursor_t *name_cursor(const mu_name_t *name) {
  return (name_cursor_t *) ((char *) name - offsetof(name_header_t, name));
}

#endif /* MU_NAME_I */
