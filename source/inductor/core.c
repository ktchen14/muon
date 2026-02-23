#include "core.h"

#include "common.h"
#include "induce.h"
#include "../engine.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/// @internal Return the mutable inductor of the @a core
static inline MuonInductor *unlock_inductor(struct MuonCore *core) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
  return (MuonInductor *) core->induce;
#pragma GCC diagnostic pop
}

MuonCore *mu_simple_core(induce_t *induce, MuonName *name) {
  struct MuonCore *core;
  if ((core = malloc(sizeof(MuonCore))) == NULL)
    return NULL;
  *core = (MuonCore) {.tag = MUON_CUSTOM_CORE, .induce = induce, .name = name};
  return core;
}

MuonCore *single_record_core(induce_t *induce, MuonName *name) {
  for (size_t i = 0; i < induce->core_length; i++) {
    MuonCore *candidate = induce->core[i];
    if (candidate->tag != MUON_RECORD_CORE)
      continue;

    if (candidate->argc != 1)
      continue;

    if (candidate->argv[0].name == name)
      return candidate;
  }

  struct MuonCore *result;
  if ((result = malloc(struct_size(MuonCore, argv, 1))) == NULL)
    return NULL;

  *result = (MuonCore) {.tag = MUON_RECORD_CORE, .induce = induce, .argc = 1};
  result->argv[0] = (MuonCoreMember) {.name = name};

  induce->core[induce->core_length++] = result;
  return result;
}

const mu_instance_t *mu_instance(
    MuonCore *source, MuonCore *target, MuonExpr *expr) {
  mu_instance_t *instance;
  if ((instance = malloc(sizeof(mu_instance_t))) == NULL)
    return NULL;
  *instance = (mu_instance_t) {
    .source = source, .target = target, .expr = expr
  };
  return instance;
}

struct MuonCore *record_core_allocate(induce_t *induce, size_t argc) {
  size_t size;
  if (rare((size = struct_size(MuonCore, argv, argc)) == 0))
    return NULL;

  struct MuonCore *allocation;
  if ((allocation = malloc(size)) == NULL)
    return NULL;
  *allocation = (MuonCore) {
    .tag = MUON_RECORD_CORE, .induce = induce, .argc = argc
  };
  return allocation;
}

MuonCore *record_core_activate(struct MuonCore *allocation) {
  // Ensure that each member is sorted after the previous one
  for (size_t i = 1; i < allocation->argc; i++)
    assert(name_cmp(allocation->argv[i].name, allocation->argv[i - 1].name) > 0);

  MuonInductor *inductor = unlock_inductor(allocation);

  for (size_t i = 0; i < inductor->core_length; i++) {
    MuonCore *core = inductor->core[i];
    if (core->tag != MUON_RECORD_CORE)
      continue;

    if (allocation->argc != core->argc)
      continue;

    for (size_t j = 0; j < allocation->argc; j++) {
      if (core->argv[j].name != allocation->argv[j].name)
        goto next;
    }

    free(allocation);
    return core;

  next:;
  }

  return inductor->core[inductor->core_length++] = allocation;
}

const record_instance_t *get_record_instance(
    induce_t *induce, MuonCore *source, MuonCore *target) {
  assert(source->tag == MUON_RECORD_CORE);
  assert(target->tag == MUON_RECORD_CORE);

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

void mu_core_debug(MuonCore *core) {
  static const char *const VARIANCE_TEXT[] = {
    [MUON_COVARIANCE] = "+", [MUON_CONTRAVARIANCE] = "+", [MUON_INVARIANCE] = "±"
  };

  if (debug_shortcore) {
    switch (core->tag) {
      case MUON_BOOLEAN_CORE:
        debug(PRIsKIND, DEBUG_CORE_KIND("𝔹"));
        return;
      case MUON_INTEGER_CORE:
        debug(PRIsKIND, DEBUG_CORE_KIND("𝕀"));
        return;
      case MUON_LAMBDA_CORE:
        debug(PRIsKIND, DEBUG_CORE_KIND("λ"));
        return;
      case MUON_VECTOR_CORE:
        debug(PRIsKIND, DEBUG_CORE_KIND("𝕍"));
        return;
      case MUON_RECORD_CORE:
        debug(PRIsKIND, DEBUG_CORE_KIND("ℝ"));
        return;
      case MUON_CUSTOM_CORE:
        debug(PRIsKIND, DEBUG_CORE_KIND(DEBUG_NAME(core->name)));
        return;
    }
    __builtin_unreachable();
  }

  switch (core->tag) {
    case MUON_BOOLEAN_CORE:
      debug(PRIsKIND, DEBUG_CORE_KIND("Boolean"));
      return;
    case MUON_INTEGER_CORE:
      debug(PRIsKIND, DEBUG_CORE_KIND("Integer"));
      return;
    case MUON_LAMBDA_CORE:
      debug(PRIsKIND, DEBUG_CORE_KIND("λ"));
      return;
    case MUON_VECTOR_CORE:
      debug(PRIsKIND, DEBUG_CORE_KIND("Vector"));
      return;
    case MUON_RECORD_CORE:
      debug(PRIsKIND, DEBUG_CORE_KIND("("));

      for (size_t i = 0; i < core->argc; i++) {
        if (i > 0)
          debug(", ");
        MuonName *name = core->argv[i].name;
        const char *variance = VARIANCE_TEXT[core->argv[i].variance];
        debug(PRIsNAME ": %s", DEBUG_NAME(name), variance);
      }

      debug(PRIsKIND, DEBUG_CORE_KIND(")"));
      return;
    case MUON_CUSTOM_CORE:
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
