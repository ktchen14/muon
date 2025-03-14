#include "coercion.h"

#include <stddef.h>

#include "../stator/debug.h"

const mu_variance_coercion_t *mu_variance_coercion(
    const mu_core_t *core, const mu_coercion_t *argv[/* core->argc */]) {
  assert(core->argc == 0 && argv == NULL || core->argc != 0 && argv != NULL);

  mu_variance_coercion_t *allocation;
  if ((allocation = variance_coercion_allocate(core)) == NULL)
    return NULL;

  if (core->argc > 0)
    memcpy(allocation->argv, argv, sizeof(const mu_coercion_t *[core->argc]));

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
  mu_unjoin_coercion_t *allocation;
  if ((allocation = unjoin_coercion_allocate(argc)) == NULL)
    return NULL;

  for (size_t i = 0; i < argc; i++)
    allocation->argv[i] = argv[i];

  return unjoin_coercion_activate(allocation);
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

void mu_coercion_debug(const mu_coercion_t *coercion) {
  switch (coercion->kind) {
#define MU_EMIT(lower, upper, t) \
    case MU_##upper##_COERCION: \
      mu_##lower##_coercion_debug((const mu_##lower##_coercion_t *) coercion); \
      return;
    MU_EACH_COERCION_KIND(MU_EMIT)
#undef MU_EMIT
  }
  __builtin_unreachable();
}

void mu_id_coercion_debug(const mu_id_coercion_t *coercion) {
  fprintf(stderr, PRIsKIND, DEBUG_COERCION_KIND("IdCoercion"));
}

void mu_variance_coercion_debug(const mu_variance_coercion_t *coercion) {
  fprintf(stderr, PRIsKIND "(", DEBUG_COERCION_KIND("VarianceCoercion"));

  const mu_core_t *core = coercion->core;
  mu_core_debug(core);

  for (size_t i = 0; i < core->argc; i++) {
    fputs(", ", stderr);
    mu_coercion_debug(coercion->argv[i]);
  }
  fprintf(stderr, ")");
}

void mu_record_coercion_debug(const mu_record_coercion_t *coercion) {
  const record_instance_t *instance = coercion->instance;

  fprintf(stderr, PRIsKIND "(", DEBUG_COERCION_KIND("RecordCoercion"));
  for (size_t i = 0; i < instance->target->argc; i++) {
    if (i > 0)
      fprintf(stderr, ", ");
    fprintf(stderr, "%zu", instance->argv[i]);
  }
  fprintf(stderr, ")");
}

void mu_join_coercion_debug(const mu_join_coercion_t *coercion) {
  fprintf(stderr, PRIsKIND "(i = %zu)",
      DEBUG_COERCION_KIND("JoinCoercion"), coercion->i);
}

void mu_unjoin_coercion_debug(const mu_unjoin_coercion_t *coercion) {
  fprintf(stderr, PRIsKIND "(", DEBUG_COERCION_KIND("UnjoinCoercion"));
  for (size_t i = 0; i < coercion->argc; i++) {
    if (i > 0)
      fprintf(stderr, ", ");
    mu_coercion_debug(coercion->argv[i]);
  }
  fprintf(stderr, ")");
}
