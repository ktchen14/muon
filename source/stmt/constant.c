#include "constant.h"

#include "../common.h"
#include "../engine.h"

#include <errno.h>
#include <string.h>

static inline void *engine_allocate_node(mu_engine_t *engine, size_t size) {
  if (rare((size = struct_size(node_header_t, data, size)) == 0))
    return errno = ENOMEM, NULL;

  node_header_t *header;
  if (rare((header = engine_allocate(engine, size)) == NULL))
    return NULL;
  *header = (node_header_t) {0};

  return header->data;
}

const mu_constant_stmt_t *mu_constant_stmt(
    mu_engine_t *engine, const mu_name_t *name, const mu_expr_t *expr) {
  size_t size = sizeof(mu_constant_stmt_t);

  mu_constant_stmt_t *result;
  if ((result = engine_allocate_node(engine, size)) == NULL)
    return NULL;
  *result = (mu_constant_stmt_t) {.name = name, .expr = expr};

  return engine_assign_concrete(engine, result);
}
