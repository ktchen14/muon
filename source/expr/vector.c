#include "vector.h"

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

const mu_vector_expr_t *mu_vector_expr(
    mu_engine_t *engine, size_t argc, const mu_expr_t *argv[argc]) {
  size_t size;
  if (rare((size = struct_size(mu_vector_expr_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_vector_expr_t *result;
  if ((result = engine_allocate_node(engine, size)) == NULL)
    return NULL;
  *result = (mu_vector_expr_t) { .argc = argc };
  memcpy(&result->argv, argv, sizeof(const mu_expr_t *[argc]));

  return engine_assign_concrete(engine, result);
}
