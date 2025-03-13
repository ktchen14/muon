#ifndef MU_INDUCTOR_TACTIC_I
#define MU_INDUCTOR_TACTIC_I

#include "core.h"

#include <stddef.h>

/// Expands to emit(lower, upper, title, ...) for each kind of tactic
#define MU_EACH_TACTIC_KIND(emit, ...) \
  emit(variance, VARIANCE, Variance, ##__VA_ARGS__) \
  emit(record, RECORD, Record, ##__VA_ARGS__) \
  emit(join, JOIN, Join, ##__VA_ARGS__)

/// An enumeration over each kind of tactic, e.g. @c VARIANCE_TACTIC
typedef enum {
#define MU_EMIT(l, upper, t) upper##_TACTIC,
  MU_EACH_TACTIC_KIND(MU_EMIT)
#undef MU_EMIT
} tactic_kind_t;

/// An abstract tactic
typedef struct {
  tactic_kind_t kind;
} tactic_t;

/// The header that each concrete tactic must have
#define MU_TACTIC_HEADER tactic_t as_tactic

/// The coercion can be found using a standard variance coercion on the core
typedef struct {
  MU_TACTIC_HEADER;
  const mu_core_t *core;
} variance_tactic_t;

/// The coercion involves projection of the record
typedef struct {
  MU_TACTIC_HEADER;
  const record_instance_t *instance;
} record_tactic_t;

/// The coercion involves adding a discriminant to the type to form a join type
typedef struct {
  MU_TACTIC_HEADER;
  size_t i;
} join_tactic_t;

const variance_tactic_t *variance_tactic_create(const mu_core_t *core)
  __attribute__((malloc, nonnull));

const record_tactic_t *record_tactic_create(const record_instance_t *instance)
  __attribute__((malloc, nonnull));

const join_tactic_t *join_tactic_create(size_t i)
  __attribute__((malloc));

void tactic_debug(const tactic_t *tactic)
  __attribute__((nonnull));;

void variance_tactic_debug(const variance_tactic_t *tactic)
  __attribute__((nonnull));

void record_tactic_debug(const record_tactic_t *tactic)
  __attribute__((nonnull));

void join_tactic_debug(const join_tactic_t *tactic)
  __attribute__((nonnull));

#endif /* MU_INDUCTOR_TACTIC_I */
