#ifndef MU_INDUCTOR_COERCION_I
#define MU_INDUCTOR_COERCION_I

#include <muon/inductor/coercion.h>  // IWYU pragma: export
#include <muon/inductor/type.h>

#include "core.h"

#include <stddef.h>

struct MuonEdgeCoercion {
  MU_COERCION_HEADER;
  MuonType *source;
};

/// @internal An enumeration over each kind of coercion, e.g. @c _id_coercion_kind
enum {
#define MU_EMIT(lower, u, t) _##lower##_coercion_kind,
  MU_EACH_COERCION_KIND(MU_EMIT)
#undef MU_EMIT
};

#define INTERNAL_IS_COERCION(type, name) \
  MU_COERCION_ENUMERATOR(type):; __typeof__(type) name = _object;

#define IS_COERCION(...) INTERNAL_IS_COERCION(__VA_ARGS__)

#define nominate(name) , name

const MuonEdgeCoercion *mu_edge_coercion(
    mu_inductor_t *inductor, MuonType *target, MuonType *source)
  __attribute__((malloc, nonnull));

MuonVarianceCoercion *variance_coercion_allocate(
    mu_inductor_t *inductor, const mu_core_t *core)
  __attribute__((malloc));

const MuonVarianceCoercion *variance_coercion_activate(
    MuonVarianceCoercion *coercion, MuonType *target)
  __attribute__((nonnull));

MuonUnjoinCoercion *unjoin_coercion_allocate(
    mu_inductor_t *inductor, size_t argc)
  __attribute__((malloc));

const MuonUnjoinCoercion *unjoin_coercion_activate(
    MuonUnjoinCoercion *coercion, MuonType *target)
  __attribute__((nonnull));

MuonMeetCoercion *meet_coercion_allocate(
    mu_inductor_t *inductor, size_t argc)
  __attribute__((malloc));

const MuonMeetCoercion *meet_coercion_activate(
    MuonMeetCoercion *coercion, MuonType *target)
  __attribute__((nonnull));

#endif /* MU_INDUCTOR_COERCION_I */
