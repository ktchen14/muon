#ifndef MU_INDUCTOR_COERCION_I
#define MU_INDUCTOR_COERCION_I

#include <muon/inductor/coercion.h>  // IWYU pragma: export
#include <muon/inductor/type.h>

#include "core.h"

#include <stddef.h>

struct mu_edge_coercion_t {
  MU_COERCION_HEADER;
  const mu_type_t *source;
};

/// @internal An enumeration over each kind of coercion, e.g. @c _id_coercion_kind
enum {
#define MU_EMIT(lower, u, t) _##lower##_coercion_kind,
  MU_EACH_COERCION_KIND(MU_EMIT)
#undef MU_EMIT
};

const mu_edge_coercion_t *mu_edge_coercion(
    mu_inductor_t *inductor, const mu_type_t *target, const mu_type_t *source)
  __attribute__((malloc, nonnull));

mu_variance_coercion_t *variance_coercion_allocate(
    mu_inductor_t *inductor, const mu_core_t *core)
  __attribute__((malloc));

const mu_variance_coercion_t *variance_coercion_activate(
    mu_variance_coercion_t *coercion, const mu_type_t *target)
  __attribute__((nonnull));

mu_unjoin_coercion_t *unjoin_coercion_allocate(
    mu_inductor_t *inductor, size_t argc)
  __attribute__((malloc));

const mu_unjoin_coercion_t *unjoin_coercion_activate(
    mu_unjoin_coercion_t *coercion, const mu_type_t *target)
  __attribute__((nonnull));

mu_meet_coercion_t *meet_coercion_allocate(
    mu_inductor_t *inductor, size_t argc)
  __attribute__((malloc));

const mu_meet_coercion_t *meet_coercion_activate(
    mu_meet_coercion_t *coercion, const mu_type_t *target)
  __attribute__((nonnull));

#endif /* MU_INDUCTOR_COERCION_I */
