#include "name_sign.h"

#include "../engine.h"
#include "../stator.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

const mu_name_sign_t *mu_name_sign(
    mu_engine_t *engine, const mu_name_t *name, const mu_node_source_t *source) {
  assert(name->as_stator.engine == engine);

  size_t size = sizeof(mu_name_sign_t);

  mu_name_sign_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_name_sign_t) {
    .as_sign.kind = MU_NAME_SIGN, .name = name,
  };

  if (source != NULL)
    result->as_node.source = *source;

  return assign_node(engine, result);
}

void mu_name_sign_debug(const mu_name_sign_t *sign) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Name Sign #%zu: ", sign->as_stator.id);
  mu_name_debug(sign->name);
  putc('\n', stderr);
}
