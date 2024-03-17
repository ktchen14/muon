#include "coercion.h"

#include "../common.h"
#include "../engine.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

const void *const MU_NO_SUCH_COERCION = &MU_NO_SUCH_COERCION;

reductor_t *reductor_initialize(reductor_t *reductor, MuonInductor *induce) {
  size_t node_length = induce->node_length;

  MuonCoercion **node_to_coercion;
  if ((node_to_coercion = malloc(sizeof(MuonCoercion *[node_length]))) == NULL)
    return NULL;
  for (size_t i = 0; i < node_length; i++)
    node_to_coercion[i] = NULL;

  MuonCoercion **edge_to_coercion;
  if ((edge_to_coercion = malloc(sizeof(MuonCoercion * [induce->edge_volume])))
      == NULL)
    return NULL;
  for (size_t i = 0; i < induce->edge_volume; i++)
    edge_to_coercion[i] = NULL;

  _Bool *edge_indirect;
  if ((edge_indirect = malloc(sizeof(_Bool[induce->edge_volume]))) == NULL)
    return NULL;
  for (size_t i = 0; i < induce->edge_volume; i++)
    edge_indirect[i] = 0;

  *reductor = (reductor_t) {
    .induce = induce,
    .node_to_coercion = node_to_coercion,
    .edge_to_coercion = edge_to_coercion,
    .edge_indirect = edge_indirect,
    .edge_meta_length = induce->edge_volume,
  };

  MuonType *top = &as_engine(induce->engine)->object_type->as_type;

  MuonIdCoercion *id_coercion;
  if ((id_coercion = mu_id_coercion(reductor, top)) == NULL)
    return NULL;
  reductor->id_coercion = &id_coercion->as_coercion;

  MuonSlotCoercion *slot_coercion;
  if ((slot_coercion = mu_slot_coercion(reductor, top)) == NULL)
    return NULL;
  reductor->slot_coercion = &slot_coercion->as_coercion;

  return reductor;
}

static inline mu_inductor_t *unlock_inductor(MuonCoercion *coercion) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
  return (mu_inductor_t *) coercion->inductor;
#pragma GCC diagnostic pop
}

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
    .as_coercion = {.tag = MU_ID_COERCION, .target = target},
  };
  return assign_coercion(inductor, &result->as_coercion), result;
}

MuonSlotCoercion *mu_slot_coercion(mu_inductor_t *inductor, MuonType *target) {
  struct MuonSlotCoercion *result;
  if ((result = malloc(sizeof(MuonSlotCoercion))) == NULL)
    return NULL;
  *result = (MuonSlotCoercion) {
    .as_coercion = {.tag = MU_SLOT_COERCION, .target = target},
  };
  return assign_coercion(inductor, &result->as_coercion), result;
}

MuonEdgeCoercion *mu_edge_coercion(
    mu_inductor_t *inductor, MuonType *target, MuonType *source) {
  struct MuonEdgeCoercion *result;
  if ((result = malloc(sizeof(MuonEdgeCoercion))) == NULL)
    return NULL;
  *result = (MuonEdgeCoercion) {
    .as_coercion = {.tag = MU_EDGE_COERCION, .target = target},
    .source = source,
  };
  return assign_coercion(inductor, &result->as_coercion), result;
}

MuonIndirectCoercion *mu_indirect_coercion(
    mu_inductor_t *inductor, MuonCoercion *head, MuonCoercion *tail) {
  MuonType *target = tail->target;
  if (target == NULL)
    target = head->target;

  struct MuonIndirectCoercion *result;
  if ((result = malloc(sizeof(MuonIndirectCoercion))) == NULL)
    return NULL;
  *result = (MuonIndirectCoercion) {
    .as_coercion = {.tag = MU_INDIRECT_COERCION, .target = target},
    .head = head,
    .tail = tail,
  };
  return assign_coercion(inductor, &result->as_coercion), result;
}

MuonVarianceCoercion *mu_variance_coercion(
    mu_inductor_t *inductor,
    MuonType *target,
    MuonCore *core,
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
    .as_coercion = {.tag = MU_JOIN_COERCION, .target = target},
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
    .as_coercion = {.tag = MU_UNMEET_COERCION, .target = target},
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
    .as_coercion = {.tag = MU_UNSCHEME_COERCION, .target = target},
  };
  return assign_coercion(inductor, &result->as_coercion), result;
}

struct MuonVarianceCoercion *variance_coercion_allocate(
    mu_inductor_t *inductor, MuonCore *core) {
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
    .as_coercion = {.tag = MU_VARIANCE_COERCION, .target = target},
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
    .as_coercion = {.tag = MU_UNJOIN_COERCION, .target = target},
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
    .as_coercion = {.tag = MU_MEET_COERCION, .target = target},
    .argc = coercion->argc,
  };
  memcpy(coercion, &source, offsetof(MuonMeetCoercion, argv));
  return assign_coercion(inductor, &coercion->as_coercion), coercion;
}

void mu_coercion_debug(MuonCoercion *coercion) {
  static const char *KIND_TEXT[] = {
#define MU_EMIT(l, upper, title) [MU_##upper##_COERCION] = #title,
    MU_EACH_COERCION_KIND(MU_EMIT)
#undef MU_EMIT
  };
  KIND_TEXT[MU_EDGE_COERCION] = "";
  KIND_TEXT[MU_INDIRECT_COERCION] = "";
  KIND_TEXT[MU_VARIANCE_COERCION] = "∇";
  const char *kind = KIND_TEXT[coercion->tag];

  debug(PRIsKIND, DEBUG_COERCION_KIND(kind));

  switch ON_ABSTRACT_OBJECT(coercion) {
    case MU_ID_COERCION:
      return;

    case IS_COERCION(MuonEdgeCoercion * nominate(edge_coercion)) {
      debug(PRIsKIND, DEBUG_COERCION_KIND("⟨"));
      muon_type_debug(edge_coercion->source);
      debug(" ");
      debug(PRIsKIND, DEBUG_COERCION_KIND("⇒"));
      debug(" ");
      muon_type_debug(edge_coercion->as_coercion.target);
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

    case IS_COERCION(MuonVarianceCoercion * nominate(variance_coercion))
      debug("(");
      MuonCore *core = variance_coercion->core;
      muon_core_debug(core);

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
