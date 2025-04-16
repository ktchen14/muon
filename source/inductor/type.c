#include "type.h"

#include "../common.h"
#include "../stator/name.h"
#include "core.h"
#include "induce.h"
#include "universe.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

/// @internal Allocate a type of size @a size in the @a inductor
__attribute__((malloc, nonnull))
static inline void *type_allocate(mu_inductor_t *inductor, size_t size) {
  if (rare((size = struct_size(type_header_t, data, size)) == 0))
    return errno = ENOMEM, NULL;

  type_header_t *header;
  if ((header = malloc(size)) == NULL)
    return NULL;
  *header = (type_header_t) {0};

  return header->data;
}

/// @internal Assign the abstract @a type to the @a inductor
__attribute__((nonnull, returns_nonnull))
static inline mu_type_t *assign_type(mu_inductor_t *inductor, mu_type_t *type) {
  type->induce = inductor;
  type->id = inductor->type_number++;
  return type;
}

const mu_core_type_t *mu_core_type(
    mu_inductor_t *inductor,
    const mu_core_t *core,
    const mu_type_t *const argv[/* core->argc */]) {
  mu_core_type_t *result;
  if ((result = core_type_allocate(inductor, core)) == NULL)
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

const mu_variable_type_t *mu_variable_type(induce_t *induce) {
  mu_variable_type_t *result;
  if ((result = type_allocate(induce, sizeof(mu_variable_type_t))) == NULL)
    return NULL;
  *result = (mu_variable_type_t) { .as_type.kind = MU_VARIABLE_TYPE };
  return assign_type(induce, &result->as_type), result;
}

mu_scheme_t *mu_scheme(mu_scheme_t *parent) {
  const induce_t *induce = parent->induce;

  mu_scheme_t *result;
  if ((result = malloc(sizeof(mu_scheme_t))) == NULL)
    return NULL;
  *result = (mu_scheme_t) {
    .induce = induce, .parent = parent, .id = induce->type_number,
  };
  return result;
}

const mu_scheme_type_t *mu_scheme_type(
    induce_t *induce,
    const mu_type_t *matter,
    size_t argc,
    const mu_type_t *const argv[argc]) {
  assert(argc == 0 || argv != NULL);

  mu_scheme_type_t *allocation;
  if ((allocation = scheme_type_allocate(induce, argc)) == NULL)
    return NULL;
  for (size_t i = 0; i < argc; i++)
    allocation->argv[i] = argv[i];
  return scheme_type_activate(allocation, matter);
}

mu_core_type_t *core_type_allocate(induce_t *induce, const mu_core_t *core) {
  assert(core->induce == induce);

  size_t size;
  if (rare((size = struct_size(mu_core_type_t, argv, core->argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_core_type_t *result;
  if ((result = type_allocate(induce, size)) == NULL)
    return NULL;
  *result = (mu_core_type_t) {
    .as_type = { .kind = MU_CORE_TYPE, .induce = induce }, .core = core,
  };
  return result;
}

const mu_core_type_t *core_type_activate(mu_core_type_t *type) {
  mu_inductor_t *inductor = (mu_inductor_t *) type->as_type.induce;

  for (size_t i = 0; i < type->core->argc; i++) {
    const mu_type_t *argument = type->argv[i];
    assert(argument != NULL);
    assert(argument->induce == type->as_type.induce);
  }
  return assign_type(inductor, &type->as_type), type;
}

mu_scheme_type_t *scheme_type_allocate(induce_t *induce, size_t argc) {
  size_t size;
  if (rare((size = struct_size(mu_scheme_type_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_scheme_type_t *result;
  if ((result = type_allocate(induce, size)) == NULL)
    return NULL;
  *result = (mu_scheme_type_t) {
    .as_type = { .kind = MU_SCHEME_TYPE, .induce = induce }, .argc = argc,
  };
  return result;
}

const mu_scheme_type_t *scheme_type_activate(
    mu_scheme_type_t *type, const mu_type_t *matter) {
  induce_t *induce = (induce_t *) type->as_type.induce;

  for (size_t i = 0; i < type->argc; i++) {
    assert(type->argv[i] != NULL);
    assert(type->argv[i]->induce == induce);
  }
  type->matter = matter;
  return assign_type(induce, &type->as_type), type;
}

mu_join_type_t *join_type_allocate(induce_t *induce, size_t argc) {
  size_t size;
  if (rare((size = struct_size(mu_join_type_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_join_type_t *result;
  if ((result = type_allocate(induce, size)) == NULL)
    return NULL;
  *result = (mu_join_type_t) {
    .as_type = { .kind = MU_JOIN_TYPE, .induce = induce }, .argc = argc,
  };
  return result;
}

const mu_join_type_t *join_type_activate(mu_join_type_t *type) {
  induce_t *induce = (induce_t *) type->as_type.induce;

  for (size_t i = 0; i < type->argc; i++) {
    assert(type->argv[i] != NULL);
    assert(type->argv[i]->induce == induce);
  }
  return assign_type(induce, &type->as_type), type;
}

static void type_debug_internal(const mu_type_t *type, _Bool expand, unsigned char prec, int assoc);

static void debug_variable_type_name(const mu_variable_type_t *type) {
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

  debug("%s", name);
}

// NOLINTNEXTLINE(misc-no-recursion)
static void type_debug_internal(const mu_type_t *type, _Bool expand, unsigned char prec, int assoc) {
  switch ON_ABSTRACT_OBJECT(type) {
    case IS_KIND_OF(core_type): {
      const mu_core_t *core = core_type->core;

      switch (core->kind) {
        case MU_BOOLEAN_CORE:
        case MU_INTEGER_CORE:
        case MU_CUSTOM_CORE:
          mu_core_debug(core);
          break;

        case MU_LAMBDA_CORE:
          if (prec > 1 || prec == 1 && assoc == 1)
            debug("(");

          WITH_DEBUG_NEGATE() { type_debug_internal(core_type->argv[0], expand, 1, 1); }
          debug(" → ");
          type_debug_internal(core_type->argv[1], expand, 1, 1);

          if (prec > 1 || prec == 1 && assoc == 1)
            debug(")");
          break;

        case MU_VECTOR_CORE:
          debug("[");
          type_debug_internal(core_type->argv[0], expand, 0, 0);
          debug("]");
          break;

        case MU_RECORD_CORE:
          debug("(");
          for (size_t i = 0; i < core->argc; i++) {
            if (i > 0)
              debug(", ");
            mu_name_debug(core->argv[i].name);
            debug(": ");
            type_debug_internal(core_type->argv[i], expand, 0, 0);
          }
          debug(")");
          break;
      }
      break;
    }

    case IS_KIND_OF(variable_type): {
      if (!debug_dot && variable_type->solution != NULL) {
        type_debug_internal(variable_type->solution, expand, prec, assoc);
        break;
      }

      if (!expand || debug_induce == NULL) {
        debug_variable_type_name(variable_type);
        break;
      }

      if (prec > 2 || prec == 2 && assoc != 1)
        debug("(");

      size_t length = 0;
      if (debug_negate) {
        for (size_t i = 0; i < debug_induce->universe.length; i++) {
          type_edge_t edge = debug_induce->universe.data[i];
          if (edge.source != type)
            continue;

          if (length++)
            debug(" ⊓ ");

          const mu_variable_type_t *upper_variable_type;
          if ((upper_variable_type = mu_type_cast(edge.target, upper_variable_type)) == NULL)
            type_debug_internal(edge.target, expand, 2, 1);
          else
            debug_variable_type_name(upper_variable_type);
        }

        if (length++)
          debug(" ⊓ ");
        debug_variable_type_name(variable_type);
      } else {
        for (size_t i = 0; i < debug_induce->universe.length; i++) {
          type_edge_t edge = debug_induce->universe.data[i];
          if (edge.target != type)
            continue;

          if (length++)
            debug(" ⊔ ");

          const mu_variable_type_t *lower_variable_type;
          if ((lower_variable_type = mu_type_cast(edge.source, lower_variable_type)) == NULL)
            type_debug_internal(edge.source, expand, 2, 1);
          else
            debug_variable_type_name(lower_variable_type);
        }

        if (length++)
          debug(" ⊔ ");
        debug_variable_type_name(variable_type);
      }

      if (prec > 2 || prec == 2 && assoc != 1)
        debug(")");
      break;
    }

    case IS_KIND_OF(scheme_type):
      debug("∀(");

      _Bool seen = 0;
      for (size_t i = 0; i < scheme_type->argc; i++) {
        const mu_variable_type_t *v;
        if ((v = mu_type_cast(scheme_type->argv[i], v)) == NULL)
          continue;

        if (seen)
          debug(", ");
        seen = 1;
        debug_variable_type_name(v);
      }
      debug(": ");

      type_debug_internal(scheme_type->matter, expand, 0, 0);
      debug(")");
      break;

    case IS_KIND_OF(join_type):
      if (join_type->argc == 0)
        debug("⊥");
      else {
        debug("Join(");
        for (size_t i = 0; i < join_type->argc; i++) {
          if (i > 0)
            debug(", ");
          type_debug_internal(join_type->argv[i], expand, 0, 0);
        }
        debug(")");
      }
      break;
  }
}

void type_debug(const mu_type_t *type, _Bool expand) {
  return type_debug_internal(type, expand, 0, 0);
}
