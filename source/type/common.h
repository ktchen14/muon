#ifndef MU_TYPE_COMMON_I
#define MU_TYPE_COMMON_I

#include <muon/type/common.h>  // IWYU pragma: export

#include "../common.h"
#include "../engine.h"

#include <errno.h>
#include <stddef.h>

typedef struct {
  const mu_type_t *anterior;
  size_t i;
} type_cursor_t;

typedef struct {
  type_cursor_t cursor;
  _Alignas(max_align_t) char data[];
} type_header_t;

__attribute__((malloc, nonnull))
static inline void *type_allocate(mu_engine_t *engine, size_t size) {
  if (rare((size = struct_size(type_header_t, data, size)) == 0))
    return errno = ENOMEM, NULL;

  type_header_t *header;
  if (rare((header = engine_allocate(engine, size)) == NULL))
    return NULL;
  *header = (type_header_t) {0};

  return header->data;
}

typedef struct inductor_t inductor_t;

#endif /* MU_TYPE_COMMON_I */
