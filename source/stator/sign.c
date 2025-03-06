#include "sign.h"

#include "engine.h"
#include "node.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

const mu_boolean_sign_t *mu_boolean_sign(mu_engine_t *engine) {
  mu_boolean_sign_t *result;
  if ((result = node_allocate(engine, sizeof(mu_boolean_sign_t))) == NULL)
    return NULL;
  *result = (mu_boolean_sign_t) { .as_sign.kind = MU_BOOLEAN_SIGN };
  return assign_node(engine, result);
}

const mu_integer_sign_t *mu_integer_sign(mu_engine_t *engine) {
  mu_integer_sign_t *result;
  if ((result = node_allocate(engine, sizeof(mu_integer_sign_t))) == NULL)
    return NULL;
  *result = (mu_integer_sign_t) { .as_sign.kind = MU_INTEGER_SIGN };
  return assign_node(engine, result);
}

const mu_name_sign_t *mu_name_sign(mu_engine_t *engine, const mu_name_t *name) {
  assert(name->as_stator.engine == engine);

  mu_name_sign_t *result;
  if ((result = node_allocate(engine, sizeof(mu_name_sign_t))) == NULL)
    return NULL;
  *result = (mu_name_sign_t) { .as_sign.kind = MU_NAME_SIGN, .name = name };
  return assign_node(engine, result);
}

const mu_record_sign_t *mu_record_sign(
    mu_engine_t *engine, size_t argc, const mu_sign_member_t argv[]) {
  assert(argc == 0 && argv == NULL || argc != 0 && argv != NULL);

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

  if (argc > 0)
    memcpy(&result->argv, argv, sizeof(const mu_sign_member_t[argc]));

  return assign_node(engine, result);
}

const mu_vector_sign_t *mu_vector_sign(
    mu_engine_t *engine, const mu_sign_t *matter) {
  assert(matter->as_stator.engine == engine);

  mu_vector_sign_t *result;
  if ((result = node_allocate(engine, sizeof(mu_vector_sign_t))) == NULL)
    return NULL;
  *result = (mu_vector_sign_t) {
    .as_sign.kind = MU_VECTOR_SIGN, .matter = matter,
  };
  return assign_node(engine, result);
}


#include "debug.h"

void mu_boolean_sign_debug(const mu_boolean_sign_t *sign) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Boolean Sign #%zu\n", sign->as_stator.id);
}

void mu_integer_sign_debug(const mu_integer_sign_t *sign) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Integer Sign #%zu\n", sign->as_stator.id);
}

void mu_name_sign_debug(const mu_name_sign_t *sign) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Name Sign #%zu: ", sign->as_stator.id);
  mu_name_debug(sign->name);
  debug_node_type(&sign->as_node);
  putc('\n', stderr);
}

/// Emit debugging information on the sign @a member to the debug stream
static void sign_member_debug(mu_sign_member_t member);

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

void mu_vector_sign_debug(const mu_vector_sign_t *sign) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Vector Sign #%zu:", sign->as_stator.id);
  debug_node_type(&sign->as_node); 
  putc('\n', stderr);

  WITH_DEBUG_INDENT() { mu_sign_debug(sign->matter); }
}
