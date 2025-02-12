#include "record_sign.h"

#include "engine.h"
#include "name.h"
#include "node.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/// Emit debugging information on the sign @a member to the debug stream
static void sign_member_debug(mu_sign_member_t member);

const mu_record_sign_t *mu_record_sign(
    mu_engine_t *engine, size_t argc, const mu_sign_member_t argv[argc]) {
  for (size_t i = 0; i < argc; i++) {
    const mu_name_t *member_name = argv[i].name;
    const mu_sign_t *member_sign = argv[i].sign;
    assert(member_name == NULL || member_name->as_stator.engine == engine);
    assert(member_sign != NULL);
    assert(member_sign->as_stator.engine == engine);
  }

  size_t size;
  if (rare((size = struct_size(mu_record_sign_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_record_sign_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_record_sign_t) {
    .as_sign.kind = MU_RECORD_SIGN, .argc = argc,
  };
  memcpy(&result->argv, argv, sizeof(const mu_sign_member_t[argc]));

  return assign_node(engine, result);
}

#include "debug.h"

void mu_record_sign_debug(const mu_record_sign_t *sign) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Record Sign #%zu:", sign->as_stator.id);
  debug_node_type(&sign->as_node);
  putc('\n', stderr);

  WITH_DEBUG_INDENT() {
    for (size_t i = 0; i < sign->argc; i++)
      sign_member_debug(sign->argv[i]);
  }
}

static void sign_member_debug(mu_sign_member_t member) {
  fprintf(stderr, "%*s", debug_indent, "");
  if (member.name != NULL) {
    fprintf(stderr, "Member: ");
    mu_name_debug(member.name);
    putc('\n', stderr);
  } else
    fputs("Member:\n", stderr);

  WITH_DEBUG_INDENT() { mu_sign_debug(member.sign); }
}
