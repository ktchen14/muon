#include "type.h"
#include "induce.h"

#include <stdio.h>

static void debug_variable_type_name(const type_t *type);

const type_t *boolean_type(induce_t *induce) {
  type_t *result;
  if ((result = malloc(sizeof(type_t))) == NULL)
    return NULL;
  *result = (type_t) { .kind = SIMPLE_TYPE, .core = induce->boolean_core };
  return result;
}

const type_t *integer_type(induce_t *induce) {
  type_t *result;
  if ((result = malloc(sizeof(type_t))) == NULL)
    return NULL;
  *result = (type_t) { .kind = SIMPLE_TYPE, .core = induce->integer_core };
  return result;
}

const type_t *lambda_type(induce_t *induce, const type_t *argument, const type_t *output) {
  size_t size = struct_size(type_t, argv, 2);
  assert(induce->lambda_core->argc == 2);

  type_t *result;
  if ((result = malloc(size)) == NULL)
    return NULL;
  *result = (type_t) { .kind = SIMPLE_TYPE, .core = induce->lambda_core };

  result->argv[0] = argument;
  result->argv[1] = output;

  return result;
}

const type_t *record_type(
    induce_t *induce, size_t argc, const type_member_t argv[static argc]) {
  size_t size;
  if (rare((size = struct_size(type_t, schema, argc)) == 0))
    return NULL;

  type_t *result;
  if (rare((result = malloc(size)) == NULL))
    return NULL;
  *result = (type_t) { .kind = RECORD_TYPE, .argc = argc };
  for (size_t i = 0; i < argc; i++)
    result->schema[i] = argv[i];
  return result;
};

type_t *record_type_allocate(induce_t *induce, size_t argc) {
  size_t size;
  if (rare((size = struct_size(type_t, schema, argc)) == 0))
    return NULL;

  type_t *result;
  if (rare((result = malloc(size)) == NULL))
    return NULL;
  *result = (type_t) { .kind = RECORD_TYPE, .argc = argc };
  return result;
}

const type_t *record_type_activate(type_t *type) {
  return type;
}

const type_t *vector_type(induce_t *induce, const type_t *matter) {
  size_t size = struct_size(type_t, argv, 1);
  assert(induce->vector_core->argc == 1);

  type_t *result;
  if ((result = malloc(size)) == NULL)
    return NULL;
  *result = (type_t) { .kind = SIMPLE_TYPE, .core = induce->vector_core };
  result->argv[0] = matter;

  return result;
}

const type_t *join_type(induce_t *induce, size_t argc, const type_t *argv[]) {
  size_t size;
  if (rare((size = struct_size(type_t, join_argv, argc)) == 0))
    return NULL;

  type_t *result;
  if ((result = malloc(size)) == NULL)
    return NULL;
  *result = (type_t) { .kind = JOIN_TYPE, .join_argc = argc };
  for (size_t i = 0; i < argc; i++)
    result->join_argv[i] = argv[i];
  return result;
}

type_t *join_type_allocate(induce_t *induce, size_t argc) {
  size_t size;
  if (rare((size = struct_size(type_t, join_argv, argc)) == 0))
    return NULL;

  type_t *result;
  if (rare((result = malloc(size)) == NULL))
    return NULL;
  *result = (type_t) { .kind = JOIN_TYPE, .argc = argc };
  return result;
}

const type_t *join_type_activate(type_t *type) {
  return type;
}

void debug_type(const type_t *type) {
  extern _Thread_local _Bool debug_negate;

  switch (type->kind) {
    case SIMPLE_TYPE:
      switch (type->core->kind) {
        case MU_BOOLEAN_CORE:
          fprintf(stderr, "Boolean"); break;

        case MU_INTEGER_CORE:
          fprintf(stderr, "Integer"); break;

        case MU_LAMBDA_CORE:
        {
          _Bool n = debug_negate;
          debug_negate = !debug_negate;
          debug_type(type->argv[0]);
          debug_negate = n;
          fprintf(stderr, " -> ");
          debug_type(type->argv[1]);
          break;
        }

        case MU_VECTOR_CORE:
          fprintf(stderr, "[");
          debug_type(type->argv[0]);
          fprintf(stderr, "]");
          break;
      }
      break;

    case RECORD_TYPE:
      fprintf(stderr, "(");
      for (size_t i = 0; i < type->argc; i++) {
        const type_member_t *member = &type->schema[i];
        if (i > 0)
          fprintf(stderr, ", ");
        mu_name_debug(member->name);
        fprintf(stderr, ": ");
        debug_type(member->type);
      }
      fprintf(stderr, ")");
      break;

    case VARIABLE_TYPE:
      ;
      static _Atomic size_t next_number = 0;
      static const char *alphabet[] = {
        "α", "β", "γ", "δ", "ε", "ζ", "η", "θ", "ι", "κ", "μ", "ν", "ξ", "ο", "π",
        "ρ", "σ", "τ", "υ", "φ", "χ", "ψ", "ω" };
      static size_t alphabet_length = sizeof(alphabet) / sizeof(alphabet[0]);

      // Assign the variable type a number
      if (type->number == 0)
        ((type_t *) type)->number = ++next_number;

      char buffer[256];

      // Generate a name
      char *name = buffer + sizeof(buffer);
      *--name = '\0';
      for (size_t n = type->number; n-- != 0; n /= alphabet_length) {
        const char *c = alphabet[n % alphabet_length];
        memcpy(name -= strlen(c), c, strlen(c));
      }

      if (debug_induce == NULL)
        return;

      _Bool already_printed = 0;
      if (debug_negate) {
        for (size_t i = 0; i < debug_induce->sub_length; i++) {
          induce_sub_t sub = debug_induce->sub_data[i];
          if (sub.lower != type)
            continue;
          if (already_printed)
            fprintf(stderr, " ⊓ ");
          already_printed = 1;
          fprintf(stderr, "(");
          debug_type(sub.upper);
          fprintf(stderr, ")");
        }

        if (already_printed)
          fprintf(stderr, " ⊓ ");
        fprintf(stderr, "%s", name);
        if (type->positively_reachable_from_anywhere)
          fprintf(stderr, "+");
        if (type->negatively_reachable_from_anywhere)
          fprintf(stderr, "-");
      } else {
        for (size_t i = 0; i < debug_induce->sub_length; i++) {
          induce_sub_t sub = debug_induce->sub_data[i];
          if (sub.upper != type)
            continue;
          if (already_printed > 0)
            fprintf(stderr, " ⊔ ");
          already_printed = 1;
          fprintf(stderr, "(");
          debug_type(sub.lower);
          fprintf(stderr, ")");
        }

        if (already_printed)
          fprintf(stderr, " ⊔ ");
        fprintf(stderr, "%s", name);
        if (type->positively_reachable_from_anywhere)
          fprintf(stderr, "+");
        if (type->negatively_reachable_from_anywhere)
          fprintf(stderr, "-");
      }
      break;

    case SCHEME_TYPE:
      fprintf(stderr, "∀ (");
      for (size_t i = 0; i < type->polymorphic_length; i++) {
        if (i > 0)
          fprintf(stderr, ", ");
        debug_variable_type_name(type->polymorphic[i]);
      }
      fprintf(stderr, ") ");

      debug_type(type->matter);
      break;

    case JOIN_TYPE:
      if (type->join_argc == 0) {
        fprintf(stderr, "⊥");
      } else {
        for (size_t i = 0; i < type->join_argc; i++) {
          if (i > 0)
            fprintf(stderr, " ⊔ ");
          debug_type(type->join_argv[i]);
        }
      }
      break;
  }
}

static void debug_variable_type_name(const type_t *type) {
  assert(type->kind == VARIABLE_TYPE);

  static _Atomic size_t next_number = 0;
  static const char *alphabet[] = {
    "α", "β", "γ", "δ", "ε", "ζ", "η", "θ", "ι", "κ", "μ", "ν", "ξ", "ο", "π",
    "ρ", "σ", "τ", "υ", "φ", "χ", "ψ", "ω" };
  static size_t alphabet_length = sizeof(alphabet) / sizeof(alphabet[0]);

  // Assign the variable type a number
  if (type->number == 0)
    ((type_t *) type)->number = ++next_number;

  char buffer[256];

  // Generate a name
  char *name = buffer + sizeof(buffer);
  *--name = '\0';
  for (size_t n = type->number; n-- != 0; n /= alphabet_length) {
    const char *c = alphabet[n % alphabet_length];
    memcpy(name -= strlen(c), c, strlen(c));
  }

  fprintf(stderr, "%s", name);
}
