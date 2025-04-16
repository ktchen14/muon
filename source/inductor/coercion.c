#include "coercion.h"

#include "../common.h"
#include "core.h"
#include "type.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>

const void *const MU_NO_SUCH_COERCION = &MU_NO_SUCH_COERCION;

const mu_edge_coercion_t *mu_edge_coercion(const mu_type_t *source, const mu_type_t *target) {
  mu_edge_coercion_t *result;
  if ((result = malloc(sizeof(mu_edge_coercion_t))) == NULL)
    return NULL;
  *result = (mu_edge_coercion_t) {
    .as_coercion.kind = MU_EDGE_COERCION, .target = target, .source = source,
  };
  return result;
}

const mu_indirect_coercion_t *mu_indirect_coercion(
    const mu_coercion_t *head, const mu_coercion_t *tail) {
  mu_indirect_coercion_t *result;
  if ((result = malloc(sizeof(mu_indirect_coercion_t))) == NULL)
    return NULL;
  *result = (mu_indirect_coercion_t) {
    .as_coercion = { .kind = MU_INDIRECT_COERCION, },
    .head = head,
    .tail = tail,
  };
  return result;
}

const mu_instance_coercion_t *mu_instance_coercion(
    const mu_instance_t *instance) {
  mu_instance_coercion_t *result;
  if ((result = malloc(sizeof(mu_instance_coercion_t))) == NULL)
    return NULL;
  *result = (mu_instance_coercion_t) {
    .as_coercion = { .kind = MU_INSTANCE_COERCION, }, .instance = instance,
  };
  return result;
}

const mu_variance_coercion_t *mu_variance_coercion(
    const mu_core_t *core, const mu_coercion_t *argv[/* target->core->argc */]) {
  assert(core->argc == 0 || argv != NULL);

  mu_variance_coercion_t *allocation;
  if ((allocation = variance_coercion_allocate(core)) == NULL)
    return NULL;
  for (size_t i = 0; i < core->argc; i++)
    allocation->argv[i] = argv[i];
  return variance_coercion_activate(allocation);
}

const mu_join_coercion_t *mu_join_coercion(size_t i) {
  mu_join_coercion_t *result;
  if ((result = malloc(sizeof(mu_join_coercion_t))) == NULL)
    return NULL;
  *result = (mu_join_coercion_t) {
    .as_coercion.kind = MU_JOIN_COERCION, .i = i,
  };
  return result;
}

const mu_unjoin_coercion_t *mu_unjoin_coercion(
    size_t argc, const mu_coercion_t *argv[/* argc */]) {
  assert(argc == 0 || argv != NULL);

  mu_unjoin_coercion_t *allocation;
  if ((allocation = unjoin_coercion_allocate(argc)) == NULL)
    return NULL;
  for (size_t i = 0; i < argc; i++)
    allocation->argv[i] = argv[i];
  return unjoin_coercion_activate(allocation);
}

const mu_meet_coercion_t *mu_meet_coercion(
    size_t argc, const mu_coercion_t *argv[/* argc */]) {
  assert(argc == 0 || argv != NULL);

  mu_meet_coercion_t *allocation;
  if ((allocation = meet_coercion_allocate(argc)) == NULL)
    return NULL;
  for (size_t i = 0; i < argc; i++)
    allocation->argv[i] = argv[i];
  return meet_coercion_activate(allocation);
}

const mu_unmeet_coercion_t *mu_unmeet_coercion(size_t i) {
  mu_unmeet_coercion_t *result;
  if ((result = malloc(sizeof(mu_unmeet_coercion_t))) == NULL)
    return NULL;
  *result = (mu_unmeet_coercion_t) {
    .as_coercion.kind = MU_UNMEET_COERCION, .i = i,
  };
  return result;
}

const mu_unscheme_coercion_t *mu_unscheme_coercion(void) {
  mu_unscheme_coercion_t *result;
  if ((result = malloc(sizeof(mu_unscheme_coercion_t))) == NULL)
    return NULL;
  *result = (mu_unscheme_coercion_t) {
    .as_coercion.kind = MU_UNSCHEME_COERCION,
  };
  return result;
}

mu_variance_coercion_t *variance_coercion_allocate(const mu_core_t *core) {
  size_t size;
  if (rare((size = struct_size(mu_variance_coercion_t, argv, core->argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_variance_coercion_t *allocation;
  if ((allocation = malloc(size)) == NULL)
    return NULL;

  *allocation = (mu_variance_coercion_t) {
    .as_coercion.kind = MU_VARIANCE_COERCION, .core = core,
  };
  return allocation;
}

const mu_variance_coercion_t *variance_coercion_activate(
    mu_variance_coercion_t *coercion) {
  return coercion;
}

mu_unjoin_coercion_t *unjoin_coercion_allocate(size_t argc) {
  size_t size;
  if (rare((size = struct_size(mu_unjoin_coercion_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_unjoin_coercion_t *allocation;
  if ((allocation = malloc(size)) == NULL)
    return NULL;

  *allocation = (mu_unjoin_coercion_t) {
    .as_coercion.kind = MU_UNJOIN_COERCION, .argc = argc,
  };
  return allocation;
}

const mu_unjoin_coercion_t *unjoin_coercion_activate(
    mu_unjoin_coercion_t *coercion) {
  return coercion;
}

mu_meet_coercion_t *meet_coercion_allocate(size_t argc) {
  size_t size;
  if (rare((size = struct_size(mu_meet_coercion_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_meet_coercion_t *allocation;
  if ((allocation = malloc(size)) == NULL)
    return NULL;

  *allocation = (mu_meet_coercion_t) {
    .as_coercion.kind = MU_MEET_COERCION, .argc = argc,
  };
  return allocation;
}

const mu_meet_coercion_t *meet_coercion_activate(
    mu_meet_coercion_t *coercion) {
  return coercion;
}

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
      type_debug(edge_coercion->target, 0);
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
