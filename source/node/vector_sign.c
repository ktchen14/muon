#include "vector_sign.h"

#include "../engine.h"
#include "../node.h"
#include "../status.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

const mu_vector_sign_t *mu_vector_sign(
    mu_engine_t *engine, const mu_sign_t *matter, const mu_source_t *source) {
  assert(matter->as_stator.engine == engine);

  size_t size = sizeof(mu_vector_sign_t);

  mu_vector_sign_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_vector_sign_t) {
    .as_sign.kind = MU_VECTOR_SIGN, .matter = matter,
  };

  if (source != NULL)
    result->as_node.source = *source;

  return assign_node(engine, result);
}

void mu_vector_sign_debug(const mu_vector_sign_t *sign) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Vector Sign #%zu:\n", sign->as_stator.id);

  WITH_DEBUG_INDENT() { mu_sign_debug(sign->matter); }
}
