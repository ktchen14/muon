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

const record_tactic_t *record_tactic_create(
    size_t argc, size_t argv[/* argc */]) {
  record_tactic_t *allocation;
  if ((allocation = record_tactic_allocate(argc)) == NULL)
    return NULL;

  if (argc > 0)
    memcpy(allocation->argv, argv, sizeof(size_t [argc]));

  return record_tactic_activate(allocation);
}

record_tactic_t *record_tactic_allocate(size_t argc) {
  size_t size;
  if (rare((size = struct_size(record_tactic_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  record_tactic_t *allocation;
  if ((allocation = malloc(size)) == NULL)
    return NULL;

  *allocation = (record_tactic_t) {
    .as_tactic.kind = RECORD_TACTIC, .argc = argc,
  };
  return allocation;
}

const record_tactic_t *record_tactic_activate(record_tactic_t *tactic) {
  return tactic;
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
  for (size_t i = 0; i < tactic->argc; i++) {
    if (i > 0)
      fprintf(stderr, ", ");
    fprintf(stderr, "%zu", tactic->argv[i]);
  }
  fprintf(stderr, ")");
}
