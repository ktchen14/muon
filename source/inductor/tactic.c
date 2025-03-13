#include "tactic.h"

#include <stddef.h>

#include "../stator/debug.h"

const variance_tactic_t *variance_tactic_create(const mu_core_t *core) {
  variance_tactic_t *result;
  if ((result = malloc(sizeof(variance_tactic_t))) == NULL)
    return NULL;
  *result = (variance_tactic_t) {
    .as_tactic.kind = VARIANCE_TACTIC, .core = core,
  };
  return result;
}

const record_tactic_t *record_tactic_create(const record_instance_t *instance) {
  record_tactic_t *result;
  if ((result = malloc(sizeof(record_tactic_t))) == NULL)
    return NULL;
  *result = (record_tactic_t) {
    .as_tactic.kind = RECORD_TACTIC, .instance = instance
  };
  return result;
}

void tactic_debug(const tactic_t *tactic) {
  switch (tactic->kind) {
#define MU_EMIT(lower, upper, t) \
    case upper##_TACTIC: \
      lower##_tactic_debug((const lower##_tactic_t *) tactic); \
      return;
    MU_EACH_TACTIC_KIND(MU_EMIT)
#undef MU_EMIT
  }
  __builtin_unreachable();
}

void variance_tactic_debug(const variance_tactic_t *tactic) {
  fprintf(stderr, PRIsKIND "()", DEBUG_KIND("VarianceTactic"));
}

void record_tactic_debug(const record_tactic_t *tactic) {
  fprintf(stderr, PRIsKIND "(", DEBUG_KIND("RecordTactic"));
  for (size_t i = 0; i < tactic->instance->target->argc; i++) {
    if (i > 0)
      fprintf(stderr, ", ");
    fprintf(stderr, "%zu", tactic->instance->argv[i]);
  }
  fprintf(stderr, ")");
}
