#include "member.h"

#include "../engine.h"
#include "../name.h"
#include "../sign.h"
#include "../status.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

const mu_member_sign_t *mu_member_sign(
    mu_engine_t *engine,
    const mu_name_t *name,
    const mu_sign_t *matter,
    const mu_source_t *source) {
  assert(name->as_stator.engine == engine);
  assert(matter->as_stator.engine == engine);

  size_t size = sizeof(mu_member_sign_t);

  mu_member_sign_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_member_sign_t) {
    .as_sign.kind = MU_MEMBER_SIGN, .name = name, .matter = matter
  };

  if (source != NULL)
    result->as_node.source = *source;

  return engine_assign_concrete(engine, result);
}

void mu_member_sign_debug(const mu_member_sign_t *sign) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Member Sign #%zu: ", sign->as_stator.id);
  mu_name_debug(sign->name);
  putc('\n', stderr);

  WITH_DEBUG_INDENT() { mu_sign_debug(sign->matter); }
}
