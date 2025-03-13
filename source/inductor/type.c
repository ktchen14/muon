#include "type.h"
#include "induce.h"

#include <stdio.h>

/// @internal Assign the abstract @a type to the @a induce instance
__attribute__((nonnull, returns_nonnull))
static inline mu_type_t *assign_type(induce_t *induce, mu_type_t *type) {
  type->induce = induce;
  type->id = induce->type_number++;
  return type;
}

/// @internal Assign the concrete @a type to the @a induce instance
#define assign_type(induce, type) \
  ((typeof((type))) (assign_type)((induce), &(type)->as_type))

const mu_core_type_t *mu_boolean_type(induce_t *induce) {
  mu_core_type_t *result;
  if ((result = malloc(sizeof(mu_core_type_t))) == NULL)
    return NULL;
  *result = (mu_core_type_t) {
    .as_type.kind = MU_CORE_TYPE, .core = induce->boolean_core,
  };
  return assign_type(induce, result);
}

const mu_core_type_t *mu_integer_type(induce_t *induce) {
  mu_core_type_t *result;
  if ((result = malloc(sizeof(mu_core_type_t))) == NULL)
    return NULL;
  *result = (mu_core_type_t) {
    .as_type.kind = MU_CORE_TYPE, .core = induce->integer_core,
  };
  return assign_type(induce, result);
}

const mu_core_type_t *mu_lambda_type(
    induce_t *induce, const mu_type_t *argument, const mu_type_t *output) {
  assert(argument->induce == induce);
  assert(argument->kind != MU_SCHEME_TYPE);

  assert(output->induce == induce);
  assert(output->kind != MU_SCHEME_TYPE);

  // TODO: this can't overflow
  size_t size;
  if (rare((size = struct_size(mu_core_type_t, argv, 2)) == 0))
    return errno = ENOMEM, NULL;

  mu_core_type_t *result;
  if ((result = malloc(size)) == NULL)
    return NULL;

  *result = (mu_core_type_t) {
    .as_type.kind = MU_CORE_TYPE, .core = induce->lambda_core,
  };

  result->argv[0] = argument;
  result->argv[1] = output;

  return assign_type(induce, result);
}

const mu_record_type_t *mu_record_type(
    induce_t *induce, size_t argc, const mu_type_member_t argv[argc]) {
  mu_record_type_t *result;
  if ((result = record_type_allocate(induce, argc)) == NULL)
    return NULL;

  for (size_t i = 0; i < argc; i++)
    result->argv[i] = argv[i];

  return record_type_activate(result);
};

const mu_core_type_t *mu_vector_type(
    induce_t *induce, const mu_type_t *matter) {
  assert(matter->induce == induce);
  assert(matter->kind != MU_SCHEME_TYPE);

  // TODO: this can't overflow
  size_t size;
  if (rare((size = struct_size(mu_core_type_t, argv, 1)) == 0))
    return errno = ENOMEM, NULL;

  mu_core_type_t *result;
  if ((result = malloc(size)) == NULL)
    return NULL;
  *result = (mu_core_type_t) {
    .as_type.kind = MU_CORE_TYPE, .core = induce->vector_core,
  };

  result->argv[0] = matter;

  return assign_type(induce, result);
}

mu_core_type_t *core_type_allocate(induce_t *induce, const mu_core_t *core) {
  size_t size;
  if (rare((size = struct_size(mu_core_type_t, argv, core->argc)) == 0))
    return NULL;

  mu_core_type_t *result;
  if ((result = malloc(size)) == NULL)
    return NULL;
  *result = (mu_core_type_t) {
    .as_type.kind = MU_CORE_TYPE, .as_type.induce = induce, .core = core,
  };
  return result;
}

const mu_core_type_t *core_type_activate(mu_core_type_t *type) {
  for (size_t i = 0; i < type->core->argc; i++) {
    const mu_type_t *argument = type->argv[i];
    assert(argument->induce == type->as_type.induce);
    assert(argument->kind != MU_SCHEME_TYPE);
  }
  return assign_type(type->as_type.induce, type);
}

mu_record_type_t *record_type_allocate(induce_t *induce, size_t argc) {
  size_t size;
  if (rare((size = struct_size(mu_record_type_t, argv, argc)) == 0))
    return NULL;

  mu_record_type_t *result;
  if ((result = malloc(size)) == NULL)
    return NULL;
  *result = (mu_record_type_t) {
    .as_type.kind = MU_RECORD_TYPE, .as_type.induce = induce, .argc = argc,
  };
  return result;
}

const mu_record_type_t *record_type_activate(mu_record_type_t *type) {
  for (size_t i = 0; i < type->argc; i++) {
    const mu_type_member_t *member = &type->argv[i];

    assert(member->type->induce == type->as_type.induce);
    assert(member->type->kind != MU_SCHEME_TYPE);
  }
  return assign_type(type->as_type.induce, type);
}

mu_scheme_type_t *scheme_type_allocate(induce_t *induce, size_t argc) {
  size_t size;
  if (rare((size = struct_size(mu_scheme_type_t, argv, argc)) == 0))
    return NULL;

  mu_scheme_type_t *result;
  if ((result = malloc(size)) == NULL)
    return NULL;
  *result = (mu_scheme_type_t) {
    .as_type.kind = MU_SCHEME_TYPE, .as_type.induce = induce, .argc = argc,
  };
  return result;
}

const mu_scheme_type_t *scheme_type_activate(
    mu_scheme_type_t *type, const mu_type_t *matter) {
  for (size_t i = 0; i < type->argc; i++) {
    assert(type->argv[i]->as_type.induce == type->as_type.induce);
    assert(type->argv[i]->as_type.kind != MU_SCHEME_TYPE);
  }
  type->matter = matter;
  return assign_type(type->as_type.induce, type);
}

mu_join_type_t *join_type_allocate(induce_t *induce, size_t argc) {
  size_t size;
  if (rare((size = struct_size(mu_join_type_t, argv, argc)) == 0))
    return NULL;

  mu_join_type_t *result;
  if (rare((result = malloc(size)) == NULL))
    return NULL;
  *result = (mu_join_type_t) {
    .as_type.kind = MU_JOIN_TYPE, .as_type.induce = induce, .argc = argc,
  };
  return result;
}

const mu_join_type_t *join_type_activate(mu_join_type_t *type) {
  for (size_t i = 0; i < type->argc; i++) {
    assert(type->argv[i] != NULL);
    assert(type->argv[i]->induce == type->as_type.induce);
    assert(type->argv[i]->kind != MU_SCHEME_TYPE);
  }
  return assign_type(type->as_type.induce, type);
}

void debug_variable_type_name(const mu_variable_type_t *type) {
  static _Atomic size_t next_number = 0;
  static const char *alphabet[] = {
    "α", "β", "γ", "δ", "ε", "ζ", "η", "θ", "ι", "κ", "μ", "ν", "ξ", "ο", "π",
    "ρ", "σ", "τ", "υ", "φ", "χ", "ψ", "ω" };
  static size_t alphabet_length = sizeof(alphabet) / sizeof(alphabet[0]);

  // Assign the variable type a number
  if (type->number == 0)
    ((mu_variable_type_t *) type)->number = ++next_number;

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

#include "../stator/debug.h"

void debug_type(const mu_type_t *type) {
  extern _Thread_local _Bool debug_negate;

  switch (type->kind) {
    case MU_CORE_TYPE: {
      const mu_core_type_t *core_type = (const mu_core_type_t *) type;

      switch (core_type->core->kind) {
        case MU_BOOLEAN_CORE:
          fprintf(stderr, "Boolean"); break;

        case MU_INTEGER_CORE:
          fprintf(stderr, "Integer"); break;

        case MU_LAMBDA_CORE:
        {
          fprintf(stderr, "(");
          WITH_DEBUG_NEGATE() { debug_type(core_type->argv[0]); }
          fprintf(stderr, " -> ");
          debug_type(core_type->argv[1]);
          fprintf(stderr, ")");
          break;
        }

        case MU_VECTOR_CORE:
          fprintf(stderr, "[");
          debug_type(core_type->argv[0]);
          fprintf(stderr, "]");
          break;
      }
      break;
    }

    case MU_RECORD_TYPE: {
      const mu_record_type_t *record_type = (const mu_record_type_t *) type;

      fprintf(stderr, "(");
      for (size_t i = 0; i < record_type->argc; i++) {
        const mu_type_member_t *member = &record_type->argv[i];
        if (i > 0)
          fprintf(stderr, ", ");
        mu_name_debug(member->name);
        fprintf(stderr, ": ");
        debug_type(member->type);
      }
      fprintf(stderr, ")");
      break;
    }

    case MU_VARIABLE_TYPE: {
      const mu_variable_type_t *variable_type = (const mu_variable_type_t *) type;

      if (variable_type->assignment != NULL) {
        debug_type(variable_type->assignment);
        break;
      }

      if (debug_induce == NULL)
        return;

      _Bool already_printed = 0;
      if (debug_negate) {
        for (size_t i = 0; i < debug_induce->edge_length; i++) {
          induce_edge_t edge = debug_induce->edge[i];
          if (edge.lower != type)
            continue;

          const mu_variable_type_t *upper_variable_type;
          if ((upper_variable_type = mu_type_cast(edge.upper, upper_variable_type)) != NULL) {
            if (is_significant(upper_variable_type)) {
              if (already_printed)
                fprintf(stderr, " ⊓ ");
              already_printed = 1;

              debug_variable_type_name(upper_variable_type);
            }
          } else {
            if (already_printed)
              fprintf(stderr, " ⊓ ");
            already_printed = 1;

            debug_type(edge.upper);
          }
        }

        if (is_significant(variable_type)) {
          if (already_printed)
            fprintf(stderr, " ⊓ ");
          debug_variable_type_name(variable_type);
        } else if (!already_printed) {
          fprintf(stderr, "⊤");
        }
      } else {
        for (size_t i = 0; i < debug_induce->edge_length; i++) {
          induce_edge_t edge = debug_induce->edge[i];
          if (edge.upper != type)
            continue;

          const mu_variable_type_t *lower_variable_type;
          if ((lower_variable_type = mu_type_cast(edge.lower, lower_variable_type)) != NULL) {
            if (is_significant(lower_variable_type)) {
              if (already_printed > 0)
                fprintf(stderr, " ⊔ ");
              already_printed = 1;

              debug_variable_type_name(lower_variable_type);
            }
          } else {
            if (already_printed > 0)
              fprintf(stderr, " ⊔ ");
            already_printed = 1;

            debug_type(edge.lower);
          }
        }

        if (is_significant(variable_type)) {
          if (already_printed)
            fprintf(stderr, " ⊔ ");
          debug_variable_type_name(variable_type);
        } else if (!already_printed) {
          fprintf(stderr, "⊥");
        }
      }
      break;
    }

    case MU_SCHEME_TYPE: {
      const mu_scheme_type_t *scheme_type = (const mu_scheme_type_t *) type;

      fprintf(stderr, "∀(");
      for (size_t i = 0; i < scheme_type->argc; i++) {
        if (i > 0)
          fprintf(stderr, ", ");
        debug_variable_type_name(scheme_type->argv[i]);
      }
      fprintf(stderr, ") ");

      debug_type(scheme_type->matter);
      break;
    }

    case MU_JOIN_TYPE: {
      const mu_join_type_t *join_type = (const mu_join_type_t *) type;

      for (size_t i = 0; i < join_type->argc; i++) {
        if (i > 0)
          fprintf(stderr, " ⊔ ");
        debug_type(join_type->argv[i]);
      }

      if (join_type->argc == 0)
        fprintf(stderr, "⊥");

      break;
    }
  }
}

void debug_just_type(const mu_type_t *type) {
  extern _Thread_local _Bool debug_negate;

  switch (type->kind) {
    case MU_CORE_TYPE: {
      const mu_core_type_t *core_type = (const mu_core_type_t *) type;

      switch (core_type->core->kind) {
        case MU_BOOLEAN_CORE:
          fprintf(stderr, "Boolean"); break;

        case MU_INTEGER_CORE:
          fprintf(stderr, "Integer"); break;

        case MU_LAMBDA_CORE:
        {
          fprintf(stderr, "(");
          WITH_DEBUG_NEGATE() { debug_just_type(core_type->argv[0]); }
          fprintf(stderr, " -> ");
          debug_just_type(core_type->argv[1]);
          fprintf(stderr, ")");
          break;
        }

        case MU_VECTOR_CORE:
          fprintf(stderr, "[");
          debug_just_type(core_type->argv[0]);
          fprintf(stderr, "]");
          break;
      }
      break;
    }

    case MU_RECORD_TYPE: {
      const mu_record_type_t *record_type = (const mu_record_type_t *) type;

      fprintf(stderr, "(");
      for (size_t i = 0; i < record_type->argc; i++) {
        const mu_type_member_t *member = &record_type->argv[i];
        if (i > 0)
          fprintf(stderr, ", ");
        mu_name_debug(member->name);
        fprintf(stderr, ": ");
        debug_just_type(member->type);
      }
      fprintf(stderr, ")");
      break;
    }

    case MU_VARIABLE_TYPE: {
      const mu_variable_type_t *variable_type = (const mu_variable_type_t *) type;

      debug_variable_type_name(variable_type);
      break;
    }

    case MU_SCHEME_TYPE: {
      const mu_scheme_type_t *scheme_type = (const mu_scheme_type_t *) type;

      fprintf(stderr, "∀(");
      for (size_t i = 0; i < scheme_type->argc; i++) {
        if (i > 0)
          fprintf(stderr, ", ");
        debug_variable_type_name(scheme_type->argv[i]);
      }
      fprintf(stderr, ") ");

      debug_just_type(scheme_type->matter);
      break;
    }

    case MU_JOIN_TYPE: {
      const mu_join_type_t *join_type = (const mu_join_type_t *) type;

      for (size_t i = 0; i < join_type->argc; i++) {
        if (i > 0)
          fprintf(stderr, " ⊔ ");
        debug_just_type(join_type->argv[i]);
      }

      if (join_type->argc == 0)
        fprintf(stderr, "⊥");

      break;
    }
  }
}

/* const type_t *join_type(induce_t *induce, size_t argc, const type_t *argv[]) { */
/*   size_t size; */
/*   if (rare((size = struct_size(type_t, join_argv, argc)) == 0)) */
/*     return NULL; */

/*   type_t *result; */
/*   if ((result = malloc(size)) == NULL) */
/*     return NULL; */
/*   *result = (type_t) { .kind = JOIN_TYPE, .join_argc = argc }; */
/*   for (size_t i = 0; i < argc; i++) */
/*     result->join_argv[i] = argv[i]; */
/*   return result; */
/* } */
