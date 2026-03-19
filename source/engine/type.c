#include "type.h"

#include "common.h"
#include "core.h"
#include "stator.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

/// @internal Return the mutable engine of the @a type
static inline MuonEngine *unlock_engine(struct MuonType *type) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
  return (MuonEngine *) type->engine;
#pragma GCC diagnostic pop
}

/// @internal Allocate a type of size @a size in the @a engine
MUON_HINT(malloc, nonnull)
static inline void *type_allocate(MuonEngine *engine, size_t size) {
  if (rare((size = struct_size(TypeHeader, type, size)) == 0))
    return errno = ENOMEM, NULL;

  TypeHeader *header;
  if ((header = engine_allocate(engine, size)) == NULL)
    return NULL;
  *header = (TypeHeader) {};

  return header->type;
}

/// @internal Assign the abstract @a type to the @a engine
MUON_HINT(nonnull, returns_nonnull)
static inline MuonType *assign_type(MuonEngine *engine, struct MuonType *type) {
  type->id = as_engine(engine)->type_number++;
  return type;
}

MuonCoreType *muon_core_type(
    MuonEngine *engine, MuonCore *core, MuonType *const argv[]) {
  struct MuonCoreType *result;
  if ((result = core_type_allocate(engine, core)) == NULL)
    return NULL;
  for (size_t i = 0; i < core_argc(core); i++)
    result->argv[i] = argv[i];
  return core_type_activate(result);
}

MuonCoreType *muon_boolean_type(MuonEngine *opaque) {
  return muon_core_type(opaque, as_engine(opaque)->boolean_core, NULL);
}

MuonCoreType *muon_integer_type(MuonEngine *opaque) {
  return muon_core_type(opaque, as_engine(opaque)->integer_core, NULL);
}

MuonCoreType *muon_lambda_type(
    MuonEngine *opaque, MuonType *argument, MuonType *output) {
  MuonType *argv[] = {argument, output};
  return muon_core_type(opaque, as_engine(opaque)->lambda_core, argv);
}

MuonCoreType *muon_vector_type(MuonEngine *opaque, MuonType *matter) {
  MuonType *argv[] = {matter};
  return muon_core_type(opaque, as_engine(opaque)->vector_core, argv);
}

MuonJoinType *muon_join_type(
    MuonEngine *engine, size_t argc, MuonType *const argv[/* argc */]) {
  struct MuonJoinType *result;
  if ((result = join_type_allocate(engine, argc)) == NULL)
    return NULL;
  for (size_t i = 0; i < argc; i++)
    result->argv[i] = argv[i];
  return join_type_activate(result);
}

MuonMeetType *muon_meet_type(
    MuonEngine *engine, size_t argc, MuonType *const argv[/* argc */]) {
  struct MuonMeetType *result;
  if ((result = meet_type_allocate(engine, argc)) == NULL)
    return NULL;
  for (size_t i = 0; i < argc; i++)
    result->argv[i] = argv[i];
  return meet_type_activate(result);
}

MuonVariableType *muon_variable_type(MuonEngine *opaque) {
  Engine *engine = as_engine(opaque);

  struct MuonVariableType *result;
  if ((result = type_allocate(opaque, sizeof(MuonVariableType))) == NULL)
    return NULL;
  *result = (MuonVariableType) {
    .as_type = {
      .tag = MUON_VARIABLE_TYPE, .engine = opaque, .scheme = engine->scheme
    }
  };
  return assign_type(opaque, &result->as_type), result;
}

MuonSchemeType *muon_scheme_type(MuonEngine *engine, MuonType *matter) {
  struct MuonSchemeType *result;
  if ((result = scheme_type_allocate(engine)) == NULL)
    return NULL;
  return scheme_type_activate(result, matter);
}

struct MuonCoreType *core_type_allocate(MuonEngine *engine, MuonCore *core) {
  assert(core->engine == engine);

  size_t size = core_argc(core);
  if (struct_size_overflow(MuonCoreType, argv, &size))
    return errno = ENOMEM, NULL;

  MuonSchemeType *scheme = as_engine(engine)->scheme;

  struct MuonCoreType *result;
  if ((result = type_allocate(engine, size)) == NULL)
    return NULL;
  *result = (MuonCoreType) {
    .as_type = {.tag = MUON_CORE_TYPE, .engine = engine, .scheme = scheme},
    .core = core
  };
  return result;
}

MuonCoreType *core_type_activate(struct MuonCoreType *type) {
  MuonEngine *engine = unlock_engine(&type->as_type);

  for (size_t i = 0; i < core_argc(type->core); i++) {
    MuonCoreMember member = core_at(type->core, i);
    assert(type->argv[member.i] != NULL);
    assert(type->argv[member.i]->engine == engine);
  }

  Hash hash = hash_object(type->core);
  hash = hash_extend(hash, type->as_type.scheme);
  for (size_t i = 0; i < core_argc(type->core); i++)
    hash = hash_extend(hash, type->argv[core_at(type->core, i).i]);

  MuonCoreType *next;
  size_t i = 0;
  for (; (next = stator_search(engine, next, hash, &i)) != NULL; i++) {
    if (next->core != type->core)
      continue;

    if (next->as_type.scheme != type->as_type.scheme)
      continue;

    for (size_t i = 0; i < core_argc(type->core); i++) {
      MuonCoreMember member = core_at(type->core, i);
      if (next->argv[member.i] != type->argv[member.i])
        goto next;
    }

    return free(type_header(&type->as_type)), next;
  next:
  }

  type->as_type.engine = engine;
  type->as_type.id = as_engine(engine)->type_number++;

  return stator_insert(engine, type, hash, i);
}

struct MuonJoinType *join_type_allocate(MuonEngine *engine, size_t argc) {
  size_t size = argc;
  if (struct_size_overflow(MuonJoinType, argv, &size))
    return errno = ENOMEM, NULL;

  MuonSchemeType *scheme = as_engine(engine)->scheme;

  struct MuonJoinType *result;
  if ((result = type_allocate(engine, size)) == NULL)
    return NULL;
  *result = (MuonJoinType) {
    .as_type = {.tag = MUON_JOIN_TYPE, .engine = engine, .scheme = scheme},
    .argc = argc,
  };
  return result;
}

MuonJoinType *join_type_activate(struct MuonJoinType *type) {
  MuonEngine *engine = unlock_engine(&type->as_type);

  for (size_t i = 0; i < type->argc; i++) {
    assert(type->argv[i] != NULL);
    assert(type->argv[i]->engine == engine);
  }

  Hash hash = hash_object(type->as_type.scheme);
  for (size_t i = 0; i < type->argc; i++)
    hash = hash_extend(hash, type->argv[i]);

  MuonJoinType *next;
  size_t i = 0;
  for (; (next = stator_search(engine, next, hash, &i)) != NULL; i++) {
    if (next->as_type.scheme != type->as_type.scheme)
      continue;

    if (next->argc != type->argc)
      continue;

    for (size_t j = 0; j < type->argc; j++) {
      if (next->argv[j] != type->argv[j])
        goto next;
    }

    return free(type_header(&type->as_type)), next;
  next:
  }

  type->as_type.engine = engine;
  type->as_type.id = as_engine(engine)->type_number++;
  return stator_insert(engine, type, hash, i);
}

struct MuonMeetType *meet_type_allocate(MuonEngine *engine, size_t argc) {
  size_t size = argc;
  if (struct_size_overflow(MuonMeetType, argv, &size))
    return errno = ENOMEM, NULL;

  MuonSchemeType *scheme = as_engine(engine)->scheme;

  struct MuonMeetType *result;
  if ((result = type_allocate(engine, size)) == NULL)
    return NULL;
  *result = (MuonMeetType) {
    .as_type = {.tag = MUON_MEET_TYPE, .engine = engine, .scheme = scheme},
    .argc = argc,
  };
  return result;
}

MuonMeetType *meet_type_activate(struct MuonMeetType *type) {
  MuonEngine *engine = unlock_engine(&type->as_type);

  for (size_t i = 0; i < type->argc; i++) {
    assert(type->argv[i] != NULL);
    assert(type->argv[i]->engine == engine);
  }

  Hash hash = hash_object(type->as_type.scheme);
  for (size_t i = 0; i < type->argc; i++)
    hash = hash_extend(hash, type->argv[i]);

  MuonMeetType *next;
  size_t i = 0;
  for (; (next = stator_search(engine, next, hash, &i)) != NULL; i++) {
    if (next->as_type.scheme != type->as_type.scheme)
      continue;

    if (next->argc != type->argc)
      continue;

    for (size_t j = 0; j < type->argc; j++) {
      if (next->argv[j] != type->argv[j])
        goto next;
    }

    return free(type_header(&type->as_type)), next;
  next:
  }

  type->as_type.engine = engine;
  type->as_type.id = as_engine(engine)->type_number++;
  return stator_insert(engine, type, hash, i);
}

struct MuonSchemeType *scheme_type_allocate(MuonEngine *engine) {
  struct MuonSchemeType *result;
  if ((result = type_allocate(engine, sizeof(MuonSchemeType))) == NULL)
    return NULL;
  *result = (MuonSchemeType) {
    .as_type = {
      .tag = MUON_SCHEME_TYPE,
      .engine = engine,
      .scheme = as_engine(engine)->scheme,
    },
  };
  return result;
}

MuonSchemeType *scheme_type_activate(
    struct MuonSchemeType *type, MuonType *matter) {
  MuonEngine *engine = unlock_engine(&type->as_type);
  assert(matter->engine == engine);
  type->matter = matter;
  return assign_type(engine, &type->as_type), type;
}

static void debug_variable_type_name(MuonVariableType *type) {
  static const char *alphabet[] = {
    "α", "β", "γ", "δ", "ε", "ζ", "η", "θ", "ι", "κ", "μ", "ν", "ξ", "ο", "π", //-
    "ρ", "σ", "τ", "υ", "φ", "χ", "ψ", "ω", //-
  };
  static size_t alphabet_length = sizeof(alphabet) / sizeof(alphabet[0]);

  char buffer[256];

  // Generate a name
  char *name = buffer + sizeof(buffer);
  *--name = '\0';
  for (size_t n = type->as_type.id; n-- != 0; n /= alphabet_length) {
    const char *c = alphabet[n % alphabet_length];
    memcpy(name -= strlen(c), c, strlen(c));
  }

  debug("%s", name);
}

// NOLINTNEXTLINE(misc-no-recursion)
void (muon_type_debug)(MuonType *type, struct MuonTypeDebugArgs args) { //-
  struct MuonTypeDebugArgs next_args = args;

  switch ON_ABSTRACT_TYPE(type) {
    case IS_CONCRETE_TYPE(MuonCoreType *core_type)
      switch ON_ABSTRACT_CORE(core_type->core) {
        case MUON_BOOLEAN_CORE:
          debug("Boolean");
          break;

        case MUON_CUSTOM_CORE:
          debug("Custom");
          break;

        case MUON_INTEGER_CORE:
          debug("Integer");
          break;

        case MUON_LAMBDA_CORE:
          if (args.strength > 1)
            debug("(");

          next_args.strength = 2;
          (muon_type_debug)(core_type->argv[0], next_args);

          debug(" → ");

          next_args.strength = 1;
          (muon_type_debug)(core_type->argv[1], next_args);

          if (args.strength > 1)
            debug(")");
          break;

        case IS_CONCRETE_CORE(MuonRecordCore *record_core)
          next_args.strength = 0;

          debug("(");
          for (size_t i = 0; i < record_core->argc; i++) {
            if (i > 0)
              debug(", ");
            muon_name_debug(record_core->argv[i]);
            debug(": ");
            (muon_type_debug)(core_type->argv[i], next_args);
          }
          debug(")");
          break;

        case MUON_VECTOR_CORE:
          next_args.strength = 0;

          debug("[");
          (muon_type_debug)(core_type->argv[0], next_args);
          debug("]");
          break;
      }
      break;

    case IS_CONCRETE_TYPE(MuonJoinType *join_type)
      if (join_type->argc == 0) {
        debug("⊥");
        break;
      }

      if (args.strength > 3)
        debug("(");

      next_args.strength = 3;
      for (size_t i = 0; i < join_type->argc; i++) {
        if (i > 0)
          debug(" ⊔ ");
        (muon_type_debug)(join_type->argv[i], next_args);
      }

      if (args.strength > 3)
        debug(")");
      break;

    case IS_CONCRETE_TYPE(MuonMeetType *meet_type)
      if (meet_type->argc == 0) {
        debug("⊤");
        break;
      }

      if (args.strength > 4)
        debug("(");

      next_args.strength = 4;
      for (size_t i = 0; i < meet_type->argc; i++) {
        if (i > 0)
          debug(" ⊓ ");
        (muon_type_debug)(meet_type->argv[i], next_args);
      }

      if (args.strength > 4)
        debug(")");
      break;

    case IS_CONCRETE_TYPE(MuonSchemeType *scheme_type)
      debug("§%zu ", scheme_type->as_type.id);
      debug("(");
      next_args.strength = 0;
      (muon_type_debug)(scheme_type->matter, next_args);
      debug(")");
      break;

    case IS_CONCRETE_TYPE(MuonVariableType *variable_type) {
      debug("v%zu", variable_type->as_type.id);
      // debug_variable_type_name(variable_type);

      MuonSchemeType *scheme_type;
      if ((scheme_type = variable_type->as_type.scheme) != NULL)
        debug(":%zu", scheme_type->as_type.id);
    }
  }

  // debug(" {%zu}", type->id);
}
