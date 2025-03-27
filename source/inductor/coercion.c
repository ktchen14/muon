#include "coercion.h"

#include "../common.h"
#include "core.h"
#include "type.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>

const void *const NO_SUCH_COERCION = &NO_SUCH_COERCION;

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

const mu_variance_coercion_t *mu_variance_coercion(
    const mu_core_type_t *target, const mu_coercion_t *argv[/* target->core->argc */]) {
  const mu_core_t *core = target->core;
  assert(core->argc == 0 || argv != NULL);

  mu_variance_coercion_t *allocation;
  if ((allocation = variance_coercion_allocate(target)) == NULL)
    return NULL;

  for (size_t i = 0; i < core->argc; i++)
    allocation->argv[i] = argv[i];

  return variance_coercion_activate(allocation);
}

const mu_record_coercion_t *mu_record_coercion(const record_instance_t *instance) {
  mu_record_coercion_t *result;
  if ((result = malloc(sizeof(mu_record_coercion_t))) == NULL)
    return NULL;
  *result = (mu_record_coercion_t) {
    .as_coercion.kind = MU_RECORD_COERCION, .instance = instance,
  };
  return result;
}

const mu_join_coercion_t *mu_join_coercion(
    const mu_variable_type_t *target, size_t i) {
  mu_join_coercion_t *result;
  if ((result = malloc(sizeof(mu_join_coercion_t))) == NULL)
    return NULL;
  *result = (mu_join_coercion_t) {
    .as_coercion.kind = MU_JOIN_COERCION, .target = target, .i = i,
  };
  return result;
}

const mu_unjoin_coercion_t *mu_unjoin_coercion(
    size_t argc, const mu_coercion_t *argv[/* argc */]) {
  mu_unjoin_coercion_t *allocation;
  if ((allocation = unjoin_coercion_allocate(argc)) == NULL)
    return NULL;

  for (size_t i = 0; i < argc; i++)
    allocation->argv[i] = argv[i];

  return unjoin_coercion_activate(allocation);
}

mu_variance_coercion_t *variance_coercion_allocate(const mu_core_type_t *target) {
  const mu_core_t *core = target->core;

  size_t size;
  if (rare((size = struct_size(mu_variance_coercion_t, argv, core->argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_variance_coercion_t *allocation;
  if ((allocation = malloc(size)) == NULL)
    return NULL;

  *allocation = (mu_variance_coercion_t) {
    .as_coercion.kind = MU_VARIANCE_COERCION, .target = target,
  };
  return allocation;
}

const mu_variance_coercion_t *variance_coercion_activate(
    mu_variance_coercion_t *coercion) {
  return coercion;
}

mu_record_coercion_t *record_coercion_allocate(
    const record_instance_t *instance) {
  size_t argc = instance->target->argc;

  size_t size;
  if (rare((size = struct_size(mu_record_coercion_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_record_coercion_t *allocation;
  if ((allocation = malloc(size)) == NULL)
    return NULL;

  *allocation = (mu_record_coercion_t) {
    .as_coercion.kind = MU_RECORD_COERCION, .instance = instance,
  };
  return allocation;
}

const mu_record_coercion_t *record_coercion_activate(
    mu_record_coercion_t *coercion) {
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

    case IS_KIND_OF(variance_coercion):
      debug("(");
      const mu_core_t *core = variance_coercion->target->core;
      mu_core_debug(core);

      for (size_t i = 0; i < core->argc; i++) {
        debug(", ");
        mu_coercion_debug(variance_coercion->argv[i]);
      }
      debug(")");
      return;

    case IS_KIND_OF(record_coercion): {
      const record_instance_t *instance = record_coercion->instance;

      debug("(");
      for (size_t i = 0; i < instance->target->argc; i++) {
        if (i > 0)
          debug(", ");
        debug("%zu: ", instance->argv[i]);
        mu_coercion_debug(record_coercion->argv[i]);
      }
      debug(")");
      return;
    }

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
  }
  __builtin_unreachable();
}
