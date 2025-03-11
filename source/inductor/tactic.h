#ifndef MU_INDUCTOR_TACTIC_I
#define MU_INDUCTOR_TACTIC_I

#include "core.h"

#include <stddef.h>

/// Expands to emit(lower, upper, title, ...) for each kind of tactic
#define MU_EACH_TACTIC_KIND(emit, ...) \
  emit(variance, VARIANCE, Variance, ##__VA_ARGS__) \
  emit(record, RECORD, Record, ##__VA_ARGS__)

/// An enumeration over each kind of tactic, e.g. @c MU_ID_TACTIC
typedef enum {
#define MU_EMIT(l, upper, t) upper##_TACTIC,
  MU_EACH_TACTIC_KIND(MU_EMIT)
#undef MU_EMIT
} tactic_kind_t;

/// An abstract tactic
typedef struct tactic_t {
  tactic_kind_t kind;
} tactic_t;

/// The header that each concrete tactic must have
#define MU_TACTIC_HEADER tactic_t as_tactic

typedef struct {
  MU_TACTIC_HEADER;
  const mu_core_t *core;
} variance_tactic_t;

typedef struct {
  MU_TACTIC_HEADER;
  size_t argc;
  size_t argv[];
} record_tactic_t;

const variance_tactic_t *variance_tactic_create(const mu_core_t *core)
  __attribute__((malloc, nonnull));

const record_tactic_t *record_tactic_create(
    size_t argc, size_t argv[/* argc */])
  __attribute__((malloc));

record_tactic_t *record_tactic_allocate(size_t argc)
  __attribute__((malloc));

const record_tactic_t *record_tactic_activate(record_tactic_t *tactic)
  __attribute__((nonnull));

void tactic_debug(const tactic_t *tactic)
  __attribute__((nonnull));;

void variance_tactic_debug(const variance_tactic_t *tactic)
  __attribute__((nonnull));

void record_tactic_debug(const record_tactic_t *tactic)
  __attribute__((nonnull));

#endif /* MU_INDUCTOR_TACTIC_I */
