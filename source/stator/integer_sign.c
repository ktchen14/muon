#include "integer_sign.h"

#include "engine.h"
#include "node.h"

#include <stddef.h>
#include <stdio.h>

const mu_integer_sign_t *mu_integer_sign(mu_engine_t *engine) {
  size_t size = sizeof(mu_integer_sign_t);

  mu_integer_sign_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_integer_sign_t) {
    .as_sign.kind = MU_INTEGER_SIGN,
  };

  return assign_node(engine, result);
}

#include "debug.h"

void mu_integer_sign_debug(const mu_integer_sign_t *sign) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Integer Sign #%zu\n", sign->as_stator.id);
}
