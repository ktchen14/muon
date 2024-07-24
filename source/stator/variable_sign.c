#include "variable_sign.h"

#include "engine.h"
#include "node.h"

#include <stddef.h>
#include <stdio.h>

const mu_variable_sign_t *mu_variable_sign(
    mu_engine_t *engine,
    const mu_node_source_t *source) {
  size_t size = sizeof(mu_variable_sign_t);

  mu_variable_sign_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_variable_sign_t) {
    .as_sign.kind = MU_VARIABLE_SIGN,
  };

  if (source != NULL)
    result->as_node.source = *source;

  return assign_node(engine, result);
}

#include "debug.h"

void mu_variable_sign_debug(const mu_variable_sign_t *sign) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Variable Sign #%zu", sign->as_stator.id);
  debug_node_type(&sign->as_node);
  putc('\n', stderr);
}
