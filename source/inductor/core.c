#include "core.h"
#include "induce.h"

#include "../common.h"
#include "../engine/name.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

const mu_core_t *mu_simple_core(induce_t *induce, mu_name_t *name) {
  mu_core_t *core;
  if ((core = malloc(sizeof(mu_core_t))) == NULL)
    return NULL;
  *core = (mu_core_t) { .kind = MU_CUSTOM_CORE, .induce = induce, .name = name };
  return core;
}

const mu_core_t *single_record_core(induce_t *induce, mu_name_t *name) {
  for (size_t i = 0; i < induce->core_length; i++) {
    const mu_core_t *candidate = induce->core[i];
    if (candidate->kind != MU_RECORD_CORE)
      continue;

    if (candidate->argc != 1)
      continue;

    if (candidate->argv[0].name == name)
      return candidate;
  }

  mu_core_t *result;
  if ((result = malloc(struct_size(mu_core_t, argv, 1))) == NULL)
    return NULL;

  *result = (mu_core_t) {
    .kind = MU_RECORD_CORE, .induce = induce, .argc = 1,
  };
  result->argv[0] = (mu_core_member_t) { .name = name };

  induce->core[induce->core_length++] = result;
  return result;
}

const mu_instance_t *mu_instance(
    const mu_core_t *source, const mu_core_t *target, mu_expr_t *expr) {
  mu_instance_t *instance;
  if ((instance = malloc(sizeof(mu_instance_t))) == NULL)
    return NULL;
  *instance = (mu_instance_t) { .source = source, .target = target, .expr = expr };
  return instance;
}

mu_core_t *record_core_allocate(induce_t *induce, size_t argc) {
  size_t size;
  if (rare((size = struct_size(mu_core_t, argv, argc)) == 0))
    return NULL;

  mu_core_t *allocation;
  if ((allocation = malloc(size)) == NULL)
    return NULL;
  *allocation = (mu_core_t) {
    .kind = MU_RECORD_CORE, .induce = induce, .argc = argc,
  };
  return allocation;
}

const mu_core_t *record_core_activate(mu_core_t *core) {
  // Ensure that each member is sorted after the previous one
  for (size_t i = 1; i < core->argc; i++)
    assert(name_cmp(core->argv[i].name, core->argv[i - 1].name) > 0);

  induce_t *induce = (induce_t *) core->induce;

  for (size_t i = 0; i < induce->core_length; i++) {
    const mu_core_t *candidate = induce->core[i];
    if (candidate->kind != MU_RECORD_CORE)
      continue;

    if (core->argc != candidate->argc)
      continue;

    for (size_t j = 0; j < core->argc; j++) {
      if (candidate->argv[j].name != core->argv[j].name)
        goto next_record_core;
    }

    free(core);
    return candidate;

  next_record_core:;
  }

  induce->core[induce->core_length++] = core;
  return core;
}

const record_instance_t *get_record_instance(
    induce_t *induce, const mu_core_t *source, const mu_core_t *target) {
  assert(source->kind == MU_RECORD_CORE);
  assert(target->kind == MU_RECORD_CORE);

  for (size_t i = 0; i < induce->record_instance_length; i++) {
    const record_instance_t *instance = induce->record_instance[i];
    if (instance->source == source && instance->target == target)
      return instance;
  }

  size_t size;
  if (rare((size = struct_size(record_instance_t, argv, target->argc)) == 0))
    return NULL;

  record_instance_t *allocation;
  if ((allocation = malloc(size)) == NULL)
    return NULL;

  allocation->target = target;
  allocation->source = source;

  for (size_t j = 0; j < target->argc; j++) {
    for (size_t i = 0; i < source->argc; i++) {
      if (source->argv[i].name == target->argv[j].name) {
        allocation->argv[j] = i;
        goto next;
      }
    }

    fprintf(stderr, "Type mismatch\n");
    abort();
  next:;
  }

  induce->record_instance[induce->record_instance_length++] = allocation;
  return allocation;
}

void mu_core_debug(const mu_core_t *core) {
  static const char *const VARIANCE_TEXT[] = {
    [MU_COVARIANCE] = "+", [MU_CONTRAVARIANCE] = "+", [MU_INVARIANCE] = "±",
  };

  if (debug_shortcore) {
    switch (core->kind) {
      case MU_BOOLEAN_CORE:
        debug(PRIsKIND, DEBUG_CORE_KIND("𝔹")); return;
      case MU_INTEGER_CORE:
        debug(PRIsKIND, DEBUG_CORE_KIND("𝕀")); return;
      case MU_LAMBDA_CORE:
        debug(PRIsKIND, DEBUG_CORE_KIND("λ")); return;
      case MU_VECTOR_CORE:
        debug(PRIsKIND, DEBUG_CORE_KIND("𝕍")); return;
      case MU_RECORD_CORE:
        debug(PRIsKIND, DEBUG_CORE_KIND("ℝ")); return;
      case MU_CUSTOM_CORE:
        debug(PRIsKIND, DEBUG_CORE_KIND(DEBUG_NAME(core->name)));
        return;
    }
    __builtin_unreachable();
  }

  switch (core->kind) {
    case MU_BOOLEAN_CORE:
      debug(PRIsKIND, DEBUG_CORE_KIND("Boolean")); return;
    case MU_INTEGER_CORE:
      debug(PRIsKIND, DEBUG_CORE_KIND("Integer")); return;
    case MU_LAMBDA_CORE:
      debug(PRIsKIND, DEBUG_CORE_KIND("λ")); return;
    case MU_VECTOR_CORE:
      debug(PRIsKIND, DEBUG_CORE_KIND("Vector")); return;
    case MU_RECORD_CORE:
      debug(PRIsKIND, DEBUG_CORE_KIND("("));

      for (size_t i = 0; i < core->argc; i++) {
        if (i > 0)
          debug(", ");
        mu_name_t *name = core->argv[i].name;
        const char *variance = VARIANCE_TEXT[core->argv[i].variance];
        debug(PRIsNAME ": %s", DEBUG_NAME(name), variance);
      }

      debug(PRIsKIND, DEBUG_CORE_KIND(")"));
      return;
    case MU_CUSTOM_CORE:
      debug(PRIsKIND, DEBUG_CORE_KIND(DEBUG_NAME(core->name)));
      return;
  }
  __builtin_unreachable();
}

void mu_instance_debug(const mu_instance_t *instance) {
  debug("(");
  mu_core_debug(instance->source);
  debug(" → ");
  mu_core_debug(instance->target);
  debug(")");
}
