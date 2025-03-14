#ifndef MU_INDUCTOR_COERCION_I
#define MU_INDUCTOR_COERCION_I

#include "core.h"

#include <stddef.h>

/// Expands to emit(lower, upper, title, ...) for each kind of coercion
#define MU_EACH_COERCION_KIND(emit, ...) \
  emit(id, ID, Id, ##__VA_ARGS__) \
  emit(variance, VARIANCE, Variance, ##__VA_ARGS__) \
  emit(record, RECORD, Record, ##__VA_ARGS__) \
  emit(join, JOIN, Join, ##__VA_ARGS__) \
  emit(unjoin, UNJOIN, Unjoin, ##__VA_ARGS__)

/// An enumeration over each kind of coercion, e.g. @c MU_ID_COERCION
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_COERCION,
  MU_EACH_COERCION_KIND(MU_EMIT)
#undef MU_EMIT
} mu_coercion_kind_t;

/// An abstract coercion
typedef struct mu_coercion_t {
  mu_coercion_kind_t kind;
} mu_coercion_t;

/// The header that each concrete coercion must have
#define MU_COERCION_HEADER mu_coercion_t as_coercion

typedef struct {
  MU_COERCION_HEADER;
} mu_id_coercion_t;

typedef struct {
  MU_COERCION_HEADER;
  const mu_core_t *core;
  const mu_coercion_t *argv[/* core->argc */];
} mu_variance_coercion_t;

typedef struct {
  MU_COERCION_HEADER;
  const record_instance_t *instance;
} mu_record_coercion_t;

/// Coercion of τ to a join type with τ at discriminant @c i
typedef struct {
  MU_COERCION_HEADER;
  size_t i;
} mu_join_coercion_t;

/// Coercion of a join type to type τ. Each coercion in argv specifies the
/// coercion to use for that discriminant.
typedef struct {
  MU_COERCION_HEADER;
  size_t argc;
  const mu_coercion_t *argv[/* argc */];
} mu_unjoin_coercion_t;

const mu_variance_coercion_t *mu_variance_coercion(
    const mu_core_t *core, const mu_coercion_t *argv[/* core->argc */])
  __attribute__((malloc, nonnull(1)));

const mu_record_coercion_t *mu_record_coercion(const record_instance_t *instance)
  __attribute__((malloc, nonnull));

const mu_join_coercion_t *mu_join_coercion(size_t i)
  __attribute__((malloc));

const mu_unjoin_coercion_t *mu_unjoin_coercion(
    size_t argc, const mu_coercion_t *argv[/* argc */])
  __attribute__((malloc, nonnull));

mu_variance_coercion_t *variance_coercion_allocate(const mu_core_t *core)
  __attribute__((malloc));

const mu_variance_coercion_t *variance_coercion_activate(
    mu_variance_coercion_t *coercion)
  __attribute__((nonnull));

mu_unjoin_coercion_t *unjoin_coercion_allocate(size_t argc)
  __attribute__((malloc));

const mu_unjoin_coercion_t *unjoin_coercion_activate(
    mu_unjoin_coercion_t *coercion)
  __attribute__((nonnull));

void mu_coercion_debug(const mu_coercion_t *coercion)
  __attribute__((nonnull));;

void mu_id_coercion_debug(const mu_id_coercion_t *coercion)
  __attribute__((nonnull));

void mu_variance_coercion_debug(const mu_variance_coercion_t *coercion)
  __attribute__((nonnull));

void mu_record_coercion_debug(const mu_record_coercion_t *coercion)
  __attribute__((nonnull));

void mu_join_coercion_debug(const mu_join_coercion_t *coercion)
  __attribute__((nonnull));

void mu_unjoin_coercion_debug(const mu_unjoin_coercion_t *coercion)
  __attribute__((nonnull));

#endif /* MU_INDUCTOR_COERCION_I */
