#ifndef MU_NODE_COMMON_I
#define MU_NODE_COMMON_I

#include <muon/node/common.h>  // IWYU pragma: export

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

#endif /* MU_NODE_COMMON_I */
