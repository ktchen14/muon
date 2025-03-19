#include "type.h"
#include "induce.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

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

const mu_core_type_t *mu_core_type(
    induce_t *induce, const mu_core_t *core, const mu_type_t *argv[]) {
  mu_core_type_t *result;
  if ((result = core_type_allocate(induce, core)) == NULL)
    return NULL;
  for (size_t i = 0; i < core->argc; i++)
    result->argv[i] = argv[i];
  return core_type_activate(result);
}

const mu_core_type_t *mu_boolean_type(induce_t *induce) {
  return mu_core_type(induce, induce->boolean_core, NULL);
}

const mu_core_type_t *mu_integer_type(induce_t *induce) {
  return mu_core_type(induce, induce->integer_core, NULL);
}

const mu_core_type_t *mu_lambda_type(
    induce_t *induce, const mu_type_t *argument, const mu_type_t *output) {
  const mu_type_t *argv[] = { argument, output };
  return mu_core_type(induce, induce->lambda_core, argv);
}

const mu_core_type_t *mu_vector_type(
    induce_t *induce, const mu_type_t *matter) {
  const mu_type_t *argv[] = { matter };
  return mu_core_type(induce, induce->vector_core, argv);
}

mu_core_type_t *core_type_allocate(induce_t *induce, const mu_core_t *core) {
  assert(core->induce == induce);

  size_t size;
  if (rare((size = struct_size(mu_core_type_t, argv, core->argc)) == 0))
    return errno = ENOMEM, NULL;

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
    assert(argument != NULL);
    assert(argument->induce == type->as_type.induce);
    assert(argument->kind != MU_SCHEME_TYPE);
  }
  return assign_type(type->as_type.induce, type);
}

mu_scheme_type_t *scheme_type_allocate(induce_t *induce, size_t argc) {
  size_t size;
  if (rare((size = struct_size(mu_scheme_type_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

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
      const mu_core_t *core = core_type->core;

      switch (core->kind) {
        case MU_BOOLEAN_CORE:
          fprintf(stderr, "Boolean"); break;

        case MU_INTEGER_CORE:
          fprintf(stderr, "Integer"); break;

        case MU_LAMBDA_CORE:
          fprintf(stderr, "(");
          WITH_DEBUG_NEGATE() { debug_type(core_type->argv[0]); }
          fprintf(stderr, " -> ");
          debug_type(core_type->argv[1]);
          fprintf(stderr, ")");
          break;

        case MU_VECTOR_CORE:
          fprintf(stderr, "[");
          debug_type(core_type->argv[0]);
          fprintf(stderr, "]");
          break;

        case MU_RECORD_CORE:
          fprintf(stderr, "(");
          for (size_t i = 0; i < core->argc; i++) {
            if (i > 0)
              fprintf(stderr, ", ");
            mu_name_debug(core->argv[i].name);
            fprintf(stderr, ": ");
            debug_type(core_type->argv[i]);
          }
          fprintf(stderr, ")");
          break;

        case MU_CUSTOM_CORE:
          mu_core_debug(core);
      }
      break;
    }

    case MU_VARIABLE_TYPE: {
      const mu_variable_type_t *variable_type = (const mu_variable_type_t *) type;

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
  }
}

void debug_just_type(const mu_type_t *type) {
  extern _Thread_local _Bool debug_negate;

  switch (type->kind) {
    case MU_CORE_TYPE: {
      const mu_core_type_t *core_type = (const mu_core_type_t *) type;
      const mu_core_t *core = core_type->core;

      switch (core->kind) {
        case MU_BOOLEAN_CORE:
          fprintf(stderr, "Boolean"); break;

        case MU_INTEGER_CORE:
          fprintf(stderr, "Integer"); break;

        case MU_LAMBDA_CORE:
          fprintf(stderr, "(");
          WITH_DEBUG_NEGATE() { debug_just_type(core_type->argv[0]); }
          fprintf(stderr, " -> ");
          debug_just_type(core_type->argv[1]);
          fprintf(stderr, ")");
          break;

        case MU_VECTOR_CORE:
          fprintf(stderr, "[");
          debug_just_type(core_type->argv[0]);
          fprintf(stderr, "]");
          break;

        case MU_RECORD_CORE:
          fprintf(stderr, "(");
          for (size_t i = 0; i < core->argc; i++) {
            if (i > 0)
              fprintf(stderr, ", ");
            mu_name_debug(core->argv[i].name);
            fprintf(stderr, ": ");
            debug_just_type(core_type->argv[i]);
          }
          fprintf(stderr, ")");
          break;

        case MU_CUSTOM_CORE:
          mu_core_debug(core);
      }
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
  }
}
