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
static inline mu_inductor_t *unlock_inductor(MuonCoercion *coercion) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
  return (mu_inductor_t *) coercion->inductor;
#pragma GCC diagnostic pop
}

/// @internal Assign the abstract @a coercion to the @a inductor
MUON_HINT(nonnull, returns_nonnull)
static inline MuonCoercion *assign_coercion(
    mu_inductor_t *inductor, struct MuonCoercion *coercion) {
  coercion->inductor = inductor;
  coercion->id = inductor->coercion_number++;
  return coercion;
}

MuonIdCoercion *mu_id_coercion(mu_inductor_t *inductor, MuonType *target) {
  struct MuonIdCoercion *result;
  if ((result = malloc(sizeof(MuonIdCoercion))) == NULL)
    return NULL;
  *result = (MuonIdCoercion) {
    .as_coercion = {.kind = MU_ID_COERCION, .target = target},
  };
  return assign_coercion(inductor, &result->as_coercion), result;
}

MuonEdgeCoercion *mu_edge_coercion(
    mu_inductor_t *inductor, MuonType *target, MuonType *source) {
  struct MuonEdgeCoercion *result;
  if ((result = malloc(sizeof(MuonEdgeCoercion))) == NULL)
    return NULL;
  *result = (MuonEdgeCoercion) {
    .as_coercion = {.kind = MU_EDGE_COERCION, .target = target},
    .source = source,
  };
  return assign_coercion(inductor, &result->as_coercion), result;
}

MuonIndirectCoercion *mu_indirect_coercion(
    mu_inductor_t *inductor, MuonCoercion *head, MuonCoercion *tail) {
  MuonType *target = tail->target;
  // This can be NULL if the tail is an ID coercion
  if (target == NULL)
    target = head->target;

  struct MuonIndirectCoercion *result;
  if ((result = malloc(sizeof(MuonIndirectCoercion))) == NULL)
    return NULL;
  *result = (MuonIndirectCoercion) {
    .as_coercion = {.kind = MU_INDIRECT_COERCION, .target = target},
    .head = head,
    .tail = tail,
  };
  return assign_coercion(inductor, &result->as_coercion), result;
}

MuonInstanceCoercion *mu_instance_coercion(
    mu_inductor_t *inductor, MuonType *target, const mu_instance_t *instance) {
  struct MuonInstanceCoercion *result;
  if ((result = malloc(sizeof(MuonInstanceCoercion))) == NULL)
    return NULL;
  *result = (MuonInstanceCoercion) {
    .as_coercion = {.kind = MU_INSTANCE_COERCION, .target = target},
    .instance = instance,
  };
  return assign_coercion(inductor, &result->as_coercion), result;
}

MuonVarianceCoercion *mu_variance_coercion(
    mu_inductor_t *inductor,
    MuonType *target,
    const mu_core_t *core,
    MuonCoercion *argv[/* target->core->argc */]) {
  assert(core->argc == 0 || argv != NULL);

  struct MuonVarianceCoercion *allocation;
  if ((allocation = variance_coercion_allocate(inductor, core)) == NULL)
    return NULL;
  for (size_t i = 0; i < core->argc; i++)
    allocation->argv[i] = argv[i];
  return variance_coercion_activate(allocation, target);
}

MuonJoinCoercion *mu_join_coercion(
    mu_inductor_t *inductor, MuonType *target, size_t i) {
  struct MuonJoinCoercion *result;
  if ((result = malloc(sizeof(MuonJoinCoercion))) == NULL)
    return NULL;
  *result = (MuonJoinCoercion) {
    .as_coercion = {.kind = MU_JOIN_COERCION, .target = target},
    .i = i,
  };
  return assign_coercion(inductor, &result->as_coercion), result;
}

MuonUnjoinCoercion *mu_unjoin_coercion(
    mu_inductor_t *inductor,
    MuonType *target,
    size_t argc,
    MuonCoercion *argv[/* argc */]) {
  assert(argc == 0 || argv != NULL);

  struct MuonUnjoinCoercion *allocation;
  if ((allocation = unjoin_coercion_allocate(inductor, argc)) == NULL)
    return NULL;
  for (size_t i = 0; i < argc; i++)
    allocation->argv[i] = argv[i];
  return unjoin_coercion_activate(allocation, target);
}

MuonMeetCoercion *mu_meet_coercion(
    mu_inductor_t *inductor,
    MuonType *target,
    size_t argc,
    MuonCoercion *argv[/* argc */]) {
  assert(argc == 0 || argv != NULL);

  struct MuonMeetCoercion *allocation;
  if ((allocation = meet_coercion_allocate(inductor, argc)) == NULL)
    return NULL;
  for (size_t i = 0; i < argc; i++)
    allocation->argv[i] = argv[i];
  return meet_coercion_activate(allocation, target);
}

MuonUnmeetCoercion *mu_unmeet_coercion(
    mu_inductor_t *inductor, MuonType *target, size_t i) {
  struct MuonUnmeetCoercion *result;
  if ((result = malloc(sizeof(MuonUnmeetCoercion))) == NULL)
    return NULL;
  *result = (MuonUnmeetCoercion) {
    .as_coercion = {.kind = MU_UNMEET_COERCION, .target = target},
    .i = i,
  };
  return assign_coercion(inductor, &result->as_coercion), result;
}

MuonUnschemeCoercion *mu_unscheme_coercion(
    mu_inductor_t *inductor, MuonType *target) {
  struct MuonUnschemeCoercion *result;
  if ((result = malloc(sizeof(MuonUnschemeCoercion))) == NULL)
    return NULL;
  *result = (MuonUnschemeCoercion) {
    .as_coercion = {.kind = MU_UNSCHEME_COERCION, .target = target},
  };
  return assign_coercion(inductor, &result->as_coercion), result;
}

struct MuonVarianceCoercion *variance_coercion_allocate(
    mu_inductor_t *inductor, const mu_core_t *core) {
  size_t size;
  if (rare((size = struct_size(MuonVarianceCoercion, argv, core->argc)) == 0))
    return errno = ENOMEM, NULL;

  struct MuonVarianceCoercion *allocation;
  if ((allocation = malloc(size)) == NULL)
    return NULL;

  *allocation = (MuonVarianceCoercion) {
    .as_coercion.inductor = inductor, .core = core
  };
  return allocation;
}

MuonVarianceCoercion *variance_coercion_activate(
    struct MuonVarianceCoercion *coercion, MuonType *target) {
  mu_inductor_t *inductor = unlock_inductor(&coercion->as_coercion);

  for (size_t i = 0; i < coercion->core->argc; i++) {
    assert(coercion->argv[i] != NULL);
    assert(coercion->argv[i]->inductor == inductor);
  }

  MuonVarianceCoercion source = {
    .as_coercion = {.kind = MU_VARIANCE_COERCION, .target = target},
    .core = coercion->core,
  };
  memcpy(coercion, &source, offsetof(MuonVarianceCoercion, argv));
  return assign_coercion(inductor, &coercion->as_coercion), coercion;
}

struct MuonUnjoinCoercion *unjoin_coercion_allocate(
    mu_inductor_t *inductor, size_t argc) {
  size_t size;
  if (rare((size = struct_size(MuonUnjoinCoercion, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  struct MuonUnjoinCoercion *allocation;
  if ((allocation = malloc(size)) == NULL)
    return NULL;
  *allocation = (MuonUnjoinCoercion) {
    .as_coercion.inductor = inductor, .argc = argc
  };
  return allocation;
}

MuonUnjoinCoercion *unjoin_coercion_activate(
    struct MuonUnjoinCoercion *coercion, MuonType *target) {
  mu_inductor_t *inductor = unlock_inductor(&coercion->as_coercion);

  for (size_t i = 0; i < coercion->argc; i++) {
    assert(coercion->argv[i] != NULL);
    assert(coercion->argv[i]->inductor == inductor);
  }

  MuonUnjoinCoercion source = {
    .as_coercion = {.kind = MU_UNJOIN_COERCION, .target = target},
    .argc = coercion->argc,
  };
  memcpy(coercion, &source, offsetof(MuonUnjoinCoercion, argv));
  return assign_coercion(inductor, &coercion->as_coercion), coercion;
}

struct MuonMeetCoercion *meet_coercion_allocate(
    mu_inductor_t *inductor, size_t argc) {
  size_t size;
  if (rare((size = struct_size(MuonMeetCoercion, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  struct MuonMeetCoercion *allocation;
  if ((allocation = malloc(size)) == NULL)
    return NULL;

  *allocation = (MuonMeetCoercion) {
    .as_coercion.inductor = inductor, .argc = argc
  };
  return allocation;
}

MuonMeetCoercion *meet_coercion_activate(
    struct MuonMeetCoercion *coercion, MuonType *target) {
  mu_inductor_t *inductor = unlock_inductor(&coercion->as_coercion);

  for (size_t i = 0; i < coercion->argc; i++) {
    assert(coercion->argv[i] != NULL);
    assert(coercion->argv[i]->inductor == inductor);
  }

  MuonMeetCoercion source = {
    .as_coercion = {.kind = MU_MEET_COERCION, .target = target},
    .argc = coercion->argc,
  };
  memcpy(coercion, &source, offsetof(MuonMeetCoercion, argv));
  return assign_coercion(inductor, &coercion->as_coercion), coercion;
}

// NOLINTNEXTLINE(misc-no-recursion)
void mu_coercion_debug(MuonCoercion *coercion) {
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
    case MU_ID_COERCION:
      return;

    case IS_COERCION(MuonEdgeCoercion * nominate(edge_coercion)) {
      debug(PRIsKIND, DEBUG_COERCION_KIND("⟨"));
      type_debug(edge_coercion->source, 0);
      debug(" ");
      debug(PRIsKIND, DEBUG_COERCION_KIND("⇒"));
      debug(" ");
      type_debug(edge_coercion->as_coercion.target, 0);
      debug(PRIsKIND, DEBUG_COERCION_KIND("⟩"));
      return;
    }

    case IS_COERCION(MuonIndirectCoercion * nominate(indirect_coercion))
      mu_coercion_debug(indirect_coercion->head);
      debug(" ");
      debug(PRIsKIND, DEBUG_COERCION_KIND("∘"));
      debug(" ");
      mu_coercion_debug(indirect_coercion->tail);
      return;

    case IS_COERCION(MuonInstanceCoercion * nominate(instance_coercion))
      mu_instance_debug(instance_coercion->instance);
      return;

    case IS_COERCION(MuonVarianceCoercion * nominate(variance_coercion))
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

    case IS_COERCION(MuonJoinCoercion * nominate(join_coercion))
      debug("(%zu)", join_coercion->i);
      return;

    case IS_COERCION(MuonUnjoinCoercion * nominate(unjoin_coercion))
      debug("(");
      for (size_t i = 0; i < unjoin_coercion->argc; i++) {
        if (i > 0)
          debug(", ");
        mu_coercion_debug(unjoin_coercion->argv[i]);
      }
      debug(")");
      return;

    case IS_COERCION(MuonMeetCoercion * nominate(meet_coercion))
      debug("(");
      for (size_t i = 0; i < meet_coercion->argc; i++) {
        if (i > 0)
          debug(", ");
        mu_coercion_debug(meet_coercion->argv[i]);
      }
      debug(")");
      return;

    case IS_COERCION(MuonUnmeetCoercion * nominate(unmeet_coercion))
      debug("(%zu)", unmeet_coercion->i);
      return;
  }
  __builtin_unreachable();
}
