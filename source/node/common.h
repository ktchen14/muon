#ifndef MU_NODE_COMMON_I
#define MU_NODE_COMMON_I

#include <muon/node/common.h>  // IWYU pragma: export

#include "../common.h"
#include "../stator.h"

#include <errno.h>
#include <stddef.h>

typedef struct {
  const mu_node_t *node;
  size_t i;
} node_cursor_t;

typedef struct {
  node_cursor_t cursor;
  _Alignas(max_align_t) char data[];
} node_header_t;

__attribute__((const, nonnull, returns_nonnull))
static inline node_cursor_t *node_cursor(const mu_node_t *node) {
  node_header_t *header = (node_header_t *) (
      (char *) node - offsetof(node_header_t, data));
  return &header->cursor;
}

__attribute__((malloc, nonnull))
static inline void *node_allocate(mu_engine_t *engine, size_t size) {
  if (rare((size = struct_size(node_header_t, data, size)) == 0))
    return errno = ENOMEM, NULL;

  node_header_t *header;
  if (rare((header = stator_allocate(engine, size)) == NULL))
    return NULL;
  *header = (node_header_t) {0};

  return header->data;
}

#endif /* MU_NODE_COMMON_I */
