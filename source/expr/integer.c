#include "integer.h"

#include "../common.h"
#include "../engine.h"

#include <errno.h>

const mu_integer_expr_t *mu_integer_expr(mu_engine_t *engine, uint64_t data) {
  size_t size = sizeof(mu_integer_expr_t);
  if (rare((size = struct_size(node_header_t, data, size)) == 0))
    return errno = ENOMEM, NULL;

  node_header_t *header;
  if (rare((header = engine_allocate(engine, size)) == NULL))
    return NULL;
  *header = (node_header_t) {0};

  mu_integer_expr_t *integer = (mu_integer_expr_t *) &header->data;
  *integer = (mu_integer_expr_t) { .data = data };

  return engine_assign_concrete(engine, integer);
}
