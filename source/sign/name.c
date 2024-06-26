#include "name.h"

#include "../common.h"
#include "../engine.h"
#include "../name.h"
#include "../status.h"

#include <assert.h>
#include <stddef.h>

const mu_name_sign_t *mu_name_sign(
    mu_engine_t *engine, const mu_name_t *name, const mu_source_t *source) {
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

  return engine_assign_concrete(engine, result);
}
