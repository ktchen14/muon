#include "type.h"

#include "../common.h"
#include "../engine/name.h"
#include "core.h"
#include "induce.h"
#include "universe.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

/// @internal Allocate a type of size @a size in the @a inductor
MUON_HINT(malloc, nonnull)
static inline void *type_allocate(mu_inductor_t *inductor, size_t size) {
  if (rare((size = struct_size(TypeHeader, data, size)) == 0))
    return errno = ENOMEM, NULL;

  TypeHeader *header;
  if ((header = malloc(size)) == NULL)
    return NULL;
  *header = (TypeHeader) {0};

  return header->data;
}

/// @internal Assign the abstract @a type to the @a inductor
MUON_HINT(nonnull, returns_nonnull)
static inline MuonType *assign_type(
    mu_inductor_t *inductor, struct MuonType *type) {
  type->induce = inductor;
  type->id = inductor->type_number++;
  return type;
}

MuonCoreType *muon_core_type(
    mu_inductor_t *inductor,
    MuonCore *core,
    MuonType *const argv[/* core->argc */]) {
  struct MuonCoreType *result;
  if ((result = core_type_allocate(inductor, core)) == NULL)
    return NULL;
  for (size_t i = 0; i < core->argc; i++)
    result->argv[i] = argv[i];
  return core_type_activate(result);
}

MuonCoreType *muon_boolean_type(induce_t *induce) {
  return muon_core_type(induce, induce->boolean_core, NULL);
}

MuonCoreType *muon_integer_type(induce_t *induce) {
  return muon_core_type(induce, induce->integer_core, NULL);
}

MuonCoreType *muon_lambda_type(
    induce_t *induce, MuonType *argument, MuonType *output) {
  MuonType *argv[] = {argument, output};
  return muon_core_type(induce, induce->lambda_core, argv);
}

MuonCoreType *muon_vector_type(induce_t *induce, MuonType *matter) {
  MuonType *argv[] = {matter};
  return muon_core_type(induce, induce->vector_core, argv);
}

MuonVariableType *muon_variable_type(induce_t *induce) {
  struct MuonVariableType *result;
  if ((result = type_allocate(induce, sizeof(MuonVariableType))) == NULL)
    return NULL;
  *result = (MuonVariableType) {.as_type.kind = MUON_VARIABLE_TYPE};
  return assign_type(induce, &result->as_type), result;
}

mu_scheme_t *mu_scheme(mu_scheme_t *parent) {
  const induce_t *induce = parent->induce;

  mu_scheme_t *result;
  if ((result = malloc(sizeof(mu_scheme_t))) == NULL)
    return NULL;
  *result = (mu_scheme_t) {
    .induce = induce, .parent = parent, .id = induce->type_number
  };
  return result;
}

MuonSchemeType *muon_scheme_type(
    induce_t *induce,
    MuonType *matter,
    size_t argc,
    MuonType *const argv[argc]) {
  assert(argc == 0 || argv != NULL);

  struct MuonSchemeType *allocation;
  if ((allocation = scheme_type_allocate(induce, argc)) == NULL)
    return NULL;
  for (size_t i = 0; i < argc; i++)
    allocation->argv[i] = argv[i];
  return scheme_type_activate(allocation, matter);
}

struct MuonCoreType *core_type_allocate(induce_t *induce, MuonCore *core) {
  assert(core->induce == induce);

  size_t size;
  if (rare((size = struct_size(MuonCoreType, argv, core->argc)) == 0))
    return errno = ENOMEM, NULL;

  struct MuonCoreType *result;
  if ((result = type_allocate(induce, size)) == NULL)
    return NULL;
  *result = (MuonCoreType) {
    .as_type = {.kind = MUON_CORE_TYPE, .induce = induce},
    .core = core,
  };
  return result;
}

MuonCoreType *core_type_activate(struct MuonCoreType *type) {
  mu_inductor_t *inductor = (mu_inductor_t *) type->as_type.induce;

  for (size_t i = 0; i < type->core->argc; i++) {
    MuonType *argument = type->argv[i];
    assert(argument != NULL);
    assert(argument->induce == type->as_type.induce);
  }
  return assign_type(inductor, &type->as_type), type;
}

struct MuonSchemeType *scheme_type_allocate(induce_t *induce, size_t argc) {
  size_t size;
  if (rare((size = struct_size(MuonSchemeType, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  struct MuonSchemeType *result;
  if ((result = type_allocate(induce, size)) == NULL)
    return NULL;
  *result = (MuonSchemeType) {
    .as_type = {.kind = MUON_SCHEME_TYPE, .induce = induce},
    .argc = argc,
  };
  return result;
}

MuonSchemeType *scheme_type_activate(
    struct MuonSchemeType *type, MuonType *matter) {
  induce_t *induce = (induce_t *) type->as_type.induce;

  for (size_t i = 0; i < type->argc; i++) {
    assert(type->argv[i] != NULL);
    assert(type->argv[i]->induce == induce);
  }
  type->matter = matter;
  return assign_type(induce, &type->as_type), type;
}

struct MuonJoinType *join_type_allocate(induce_t *induce, size_t argc) {
  size_t size;
  if (rare((size = struct_size(MuonJoinType, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  struct MuonJoinType *result;
  if ((result = type_allocate(induce, size)) == NULL)
    return NULL;
  *result = (MuonJoinType) {
    .as_type = {.kind = MUON_JOIN_TYPE, .induce = induce},
    .argc = argc,
  };
  return result;
}

MuonJoinType *join_type_activate(struct MuonJoinType *type) {
  induce_t *induce = (induce_t *) type->as_type.induce;

  for (size_t i = 0; i < type->argc; i++) {
    assert(type->argv[i] != NULL);
    assert(type->argv[i]->induce == induce);
  }
  return assign_type(induce, &type->as_type), type;
}

static void type_debug_internal(
    MuonType *type, _Bool expand, unsigned char prec, int assoc);

static void debug_variable_type_name(MuonVariableType *type) {
  static _Atomic size_t next_number = 0;
  static const char *alphabet[] = {
    /* clang-format off */
    "α", "β", "γ", "δ", "ε", "ζ", "η", "θ", "ι", "κ", "μ", "ν", "ξ", "ο", "π",
    "ρ", "σ", "τ", "υ", "φ", "χ", "ψ", "ω",
    /* clang-format on */
  };
  static size_t alphabet_length = sizeof(alphabet) / sizeof(alphabet[0]);

  // Assign the variable type a number
  if (type->number == 0)
    ((struct MuonVariableType *) type)->number = ++next_number;

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
static void type_debug_internal(
    MuonType *type, _Bool expand, unsigned char prec, int assoc) {
  switch ON_ABSTRACT_OBJECT(type) {
    case IS_CONCRETE_TYPE(MuonCoreType *core_type) {
      MuonCore *core = core_type->core;

      switch (core->kind) {
        case MUON_BOOLEAN_CORE:
        case MUON_INTEGER_CORE:
        case MUON_CUSTOM_CORE:
          mu_core_debug(core);
          break;

        case MUON_LAMBDA_CORE:
          if (prec > 1 || prec == 1 && assoc == 1)
            debug("(");

          WITH_DEBUG_NEGATE() {
            type_debug_internal(core_type->argv[0], expand, 1, 1);
          }
          debug(" → ");
          type_debug_internal(core_type->argv[1], expand, 1, 1);

          if (prec > 1 || prec == 1 && assoc == 1)
            debug(")");
          break;

        case MUON_VECTOR_CORE:
          debug("[");
          type_debug_internal(core_type->argv[0], expand, 0, 0);
          debug("]");
          break;

        case MUON_RECORD_CORE:
          debug("(");
          for (size_t i = 0; i < core->argc; i++) {
            if (i > 0)
              debug(", ");
            muon_name_debug(core->argv[i].name);
            debug(": ");
            type_debug_internal(core_type->argv[i], expand, 0, 0);
          }
          debug(")");
          break;
      }
      break;
    }

    case IS_CONCRETE_TYPE(MuonVariableType *variable_type) {
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

          MuonVariableType *upper_variable_type;
          if ((upper_variable_type = muon_type_cast(
                   edge.target, upper_variable_type))
              == NULL)
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

          MuonVariableType *lower_variable_type;
          if ((lower_variable_type = muon_type_cast(
                   edge.source, lower_variable_type))
              == NULL)
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

    case IS_CONCRETE_TYPE(MuonSchemeType *scheme_type)
      debug("∀(");

      _Bool seen = 0;
      for (size_t i = 0; i < scheme_type->argc; i++) {
        MuonVariableType *v;
        if ((v = muon_type_cast(scheme_type->argv[i], v)) == NULL)
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

    case IS_CONCRETE_TYPE(MuonJoinType *join_type)
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

void type_debug(MuonType *type, _Bool expand) {
  type_debug_internal(type, expand, 0, 0);
}
