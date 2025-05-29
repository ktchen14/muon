#include "coercion.h"

#include "../common.h"
#include "core.h"
#include "type.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

const void *const MU_NO_SUCH_COERCION = &MU_NO_SUCH_COERCION;

/// @internal Return the mutable inductor of the @a coercion
static inline mu_inductor_t *unlock_inductor(mu_coercion_t *coercion) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
  return (mu_inductor_t *) coercion->inductor;
#pragma GCC diagnostic pop
}

/// @internal Assign the abstract @a coercion to the @a inductor
__attribute__((nonnull, returns_nonnull))
static inline mu_coercion_t *assign_coercion(
    mu_inductor_t *inductor, mu_coercion_t *coercion) {
  coercion->inductor = inductor;
  coercion->id = inductor->coercion_number++;
  return coercion;
}

const mu_edge_coercion_t *mu_edge_coercion(
    mu_inductor_t *inductor, MuonType *target, MuonType *source) {
  mu_edge_coercion_t *result;
  if ((result = malloc(sizeof(mu_edge_coercion_t))) == NULL)
    return NULL;
  *result = (mu_edge_coercion_t) {
    .as_coercion = { .kind = MU_EDGE_COERCION, .target = target },
    .source = source,
  };
  return assign_coercion(inductor, &result->as_coercion), result;
}

const mu_indirect_coercion_t *mu_indirect_coercion(
    mu_inductor_t *inductor,
    const mu_coercion_t *head, const mu_coercion_t *tail) {
  mu_indirect_coercion_t *result;
  if ((result = malloc(sizeof(mu_indirect_coercion_t))) == NULL)
    return NULL;
  *result = (mu_indirect_coercion_t) {
    .as_coercion = { .kind = MU_INDIRECT_COERCION },
    .head = head,
    .tail = tail,
  };
  return assign_coercion(inductor, &result->as_coercion), result;
}

const mu_instance_coercion_t *mu_instance_coercion(
    mu_inductor_t *inductor,
    MuonType *target,
    const mu_instance_t *instance) {
  mu_instance_coercion_t *result;
  if ((result = malloc(sizeof(mu_instance_coercion_t))) == NULL)
    return NULL;
  *result = (mu_instance_coercion_t) {
    .as_coercion = { .kind = MU_INSTANCE_COERCION, .target = target },
    .instance = instance,
  };
  return assign_coercion(inductor, &result->as_coercion), result;
}

const mu_variance_coercion_t *mu_variance_coercion(
    mu_inductor_t *inductor,
    MuonType *target,
    const mu_core_t *core,
    const mu_coercion_t *argv[/* target->core->argc */]) {
  assert(core->argc == 0 || argv != NULL);

  mu_variance_coercion_t *allocation;
  if ((allocation = variance_coercion_allocate(inductor, core)) == NULL)
    return NULL;
  for (size_t i = 0; i < core->argc; i++)
    allocation->argv[i] = argv[i];
  return variance_coercion_activate(allocation, target);
}

const mu_join_coercion_t *mu_join_coercion(
    mu_inductor_t *inductor, MuonType *target, size_t i) {
  mu_join_coercion_t *result;
  if ((result = malloc(sizeof(mu_join_coercion_t))) == NULL)
    return NULL;
  *result = (mu_join_coercion_t) {
    .as_coercion = { .kind = MU_JOIN_COERCION, .target = target }, .i = i,
  };
  return assign_coercion(inductor, &result->as_coercion), result;
}

const mu_unjoin_coercion_t *mu_unjoin_coercion(
    mu_inductor_t *inductor,
    MuonType *target,
    size_t argc,
    const mu_coercion_t *argv[/* argc */]) {
  assert(argc == 0 || argv != NULL);

  mu_unjoin_coercion_t *allocation;
  if ((allocation = unjoin_coercion_allocate(inductor, argc)) == NULL)
    return NULL;
  for (size_t i = 0; i < argc; i++)
    allocation->argv[i] = argv[i];
  return unjoin_coercion_activate(allocation, target);
}

const mu_meet_coercion_t *mu_meet_coercion(
    mu_inductor_t *inductor,
    MuonType *target,
    size_t argc,
    const mu_coercion_t *argv[/* argc */]) {
  assert(argc == 0 || argv != NULL);

  mu_meet_coercion_t *allocation;
  if ((allocation = meet_coercion_allocate(inductor, argc)) == NULL)
    return NULL;
  for (size_t i = 0; i < argc; i++)
    allocation->argv[i] = argv[i];
  return meet_coercion_activate(allocation, target);
}

const mu_unmeet_coercion_t *mu_unmeet_coercion(
    mu_inductor_t *inductor, MuonType *target, size_t i) {
  mu_unmeet_coercion_t *result;
  if ((result = malloc(sizeof(mu_unmeet_coercion_t))) == NULL)
    return NULL;
  *result = (mu_unmeet_coercion_t) {
    .as_coercion = { .kind = MU_UNMEET_COERCION, .target = target },
    .i = i,
  };
  return assign_coercion(inductor, &result->as_coercion), result;
}

const mu_unscheme_coercion_t *mu_unscheme_coercion(
    mu_inductor_t *inductor, MuonType *target) {
  mu_unscheme_coercion_t *result;
  if ((result = malloc(sizeof(mu_unscheme_coercion_t))) == NULL)
    return NULL;
  *result = (mu_unscheme_coercion_t) {
    .as_coercion = { .kind = MU_UNSCHEME_COERCION, .target = target },
  };
  return assign_coercion(inductor, &result->as_coercion), result;
}

mu_variance_coercion_t *variance_coercion_allocate(
    mu_inductor_t *inductor, const mu_core_t *core) {
  size_t size;
  if (rare((size = struct_size(mu_variance_coercion_t, argv, core->argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_variance_coercion_t *allocation;
  if ((allocation = malloc(size)) == NULL)
    return NULL;

  *allocation = (mu_variance_coercion_t) {
    .as_coercion.inductor = inductor, .core = core,
  };
  return allocation;
}

const mu_variance_coercion_t *variance_coercion_activate(
    mu_variance_coercion_t *coercion, MuonType *target) {
  mu_inductor_t *inductor = unlock_inductor(&coercion->as_coercion);

  for (size_t i = 0; i < coercion->core->argc; i++) {
    assert(coercion->argv[i] != NULL);
    assert(coercion->argv[i]->inductor == inductor);
  }

  mu_variance_coercion_t source = {
    .as_coercion = { .kind = MU_VARIANCE_COERCION, .target = target },
    .core = coercion->core,
  };
  memcpy(coercion, &source, offsetof(mu_variance_coercion_t, argv));
  return assign_coercion(inductor, &coercion->as_coercion), coercion;
}

mu_unjoin_coercion_t *unjoin_coercion_allocate(
    mu_inductor_t *inductor, size_t argc) {
  size_t size;
  if (rare((size = struct_size(mu_unjoin_coercion_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_unjoin_coercion_t *allocation;
  if ((allocation = malloc(size)) == NULL)
    return NULL;

  *allocation = (mu_unjoin_coercion_t) {
    .as_coercion.inductor = inductor, .argc = argc,
  };
  return allocation;
}

const mu_unjoin_coercion_t *unjoin_coercion_activate(
    mu_unjoin_coercion_t *coercion, MuonType *target) {
  mu_inductor_t *inductor = unlock_inductor(&coercion->as_coercion);

  for (size_t i = 0; i < coercion->argc; i++) {
    assert(coercion->argv[i] != NULL);
    assert(coercion->argv[i]->inductor == inductor);
  }

  mu_unjoin_coercion_t source = {
    .as_coercion = { .kind = MU_UNJOIN_COERCION, .target = target },
    .argc = coercion->argc,
  };
  memcpy(coercion, &source, offsetof(mu_unjoin_coercion_t, argv));
  return assign_coercion(inductor, &coercion->as_coercion), coercion;
}

mu_meet_coercion_t *meet_coercion_allocate(
    mu_inductor_t *inductor, size_t argc) {
  size_t size;
  if (rare((size = struct_size(mu_meet_coercion_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_meet_coercion_t *allocation;
  if ((allocation = malloc(size)) == NULL)
    return NULL;

  *allocation = (mu_meet_coercion_t) {
    .as_coercion.inductor = inductor, .argc = argc,
  };
  return allocation;
}

const mu_meet_coercion_t *meet_coercion_activate(
    mu_meet_coercion_t *coercion, MuonType *target) {
  mu_inductor_t *inductor = unlock_inductor(&coercion->as_coercion);

  for (size_t i = 0; i < coercion->argc; i++) {
    assert(coercion->argv[i] != NULL);
    assert(coercion->argv[i]->inductor == inductor);
  }

  mu_meet_coercion_t source = {
    .as_coercion = { .kind = MU_MEET_COERCION, .target = target },
    .argc = coercion->argc,
  };
  memcpy(coercion, &source, offsetof(mu_meet_coercion_t, argv));
  return assign_coercion(inductor, &coercion->as_coercion), coercion;
}

// NOLINTNEXTLINE(misc-no-recursion)
void mu_coercion_debug(const mu_coercion_t *coercion) {
  // Kind -> Text, e.g. [MU_ID_COERCION] = "Id"
  static const char *KIND_TEXT[] = {
#define MU_EMIT(l, upper, title) [MU_##upper##_COERCION] = #title,
    MU_EACH_COERCION_KIND(MU_EMIT)
#undef MU_EMIT
  };
  KIND_TEXT[MU_EDGE_COERCION] = "";
  KIND_TEXT[MU_INDIRECT_COERCION] = "";
  KIND_TEXT[MU_VARIANCE_COERCION] = "∇";
  const char *kind = KIND_TEXT[coercion->kind];

  debug(PRIsKIND, DEBUG_COERCION_KIND(kind));

  switch ON_ABSTRACT_OBJECT(coercion) {
    case MU_ID_COERCION: return;

    case IS_KIND_OF(edge_coercion): {
      debug(PRIsKIND, DEBUG_COERCION_KIND("⟨"));
      type_debug(edge_coercion->source, 0);
      debug(" ");
      debug(PRIsKIND, DEBUG_COERCION_KIND("⇒"));
      debug(" ");
      type_debug(edge_coercion->as_coercion.target, 0);
      debug(PRIsKIND, DEBUG_COERCION_KIND("⟩"));
      return;
    }

    case IS_KIND_OF(indirect_coercion):
      mu_coercion_debug(indirect_coercion->head);
      debug(" ");
      debug(PRIsKIND, DEBUG_COERCION_KIND("∘"));
      debug(" ");
      mu_coercion_debug(indirect_coercion->tail);
      return;

    case IS_KIND_OF(instance_coercion):
      mu_instance_debug(instance_coercion->instance);
      return;

    case IS_KIND_OF(variance_coercion):
      debug("(");
      const mu_core_t *core = variance_coercion->core;
      mu_core_debug(core);

      for (size_t i = 0; i < core->argc; i++) {
        debug(", ");
        mu_coercion_debug(variance_coercion->argv[i]);
      }
      debug(")");
      return;

    case MU_SLOT_COERCION:
      return;

    case MU_UNSCHEME_COERCION:
      return;

    case IS_KIND_OF(join_coercion):
      debug("(%zu)", join_coercion->i);
      return;

    case IS_KIND_OF(unjoin_coercion):
      debug("(");
      for (size_t i = 0; i < unjoin_coercion->argc; i++) {
        if (i > 0)
          debug(", ");
        mu_coercion_debug(unjoin_coercion->argv[i]);
      }
      debug(")");
      return;

    case IS_KIND_OF(meet_coercion):
      debug("(");
      for (size_t i = 0; i < meet_coercion->argc; i++) {
        if (i > 0)
          debug(", ");
        mu_coercion_debug(meet_coercion->argv[i]);
      }
      debug(")");
      return;

    case IS_KIND_OF(unmeet_coercion):
      debug("(%zu)", unmeet_coercion->i);
      return;
  }
  __builtin_unreachable();
}
