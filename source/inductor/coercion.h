#ifndef MU_INDUCTOR_COERCION_I
#define MU_INDUCTOR_COERCION_I

#include <muon/inductor/coercion.h> // IWYU pragma: export
#include <muon/inductor/type.h>

#include "core.h"

#include <stddef.h>

struct MuonEdgeCoercion {
  MU_COERCION_HEADER;
  MuonType *source;
};

/// @internal An enumeration over each kind of coercion, e.g.
/// @c _id_coercion_kind
enum {
#define MU_EMIT(lower, u, t) _##lower##_coercion_kind,
  MU_EACH_COERCION_KIND(MU_EMIT)
#undef MU_EMIT
};

#define INTERNAL_IS_COERCION(type, name) \
  MU_COERCION_ENUMERATOR(type):; __typeof__(type) name = _object;

#define IS_COERCION(...) INTERNAL_IS_COERCION(__VA_ARGS__)

#define nominate(name) , name

MuonEdgeCoercion *mu_edge_coercion(
    mu_inductor_t *inductor, MuonType *target, MuonType *source)
  MUON_HINT_SUFFIX(malloc, nonnull);

struct MuonVarianceCoercion *variance_coercion_allocate(
    mu_inductor_t *inductor, MuonCore *core)
  MUON_HINT_SUFFIX(malloc);

MuonVarianceCoercion *variance_coercion_activate(
    struct MuonVarianceCoercion *coercion, MuonType *target)
  MUON_HINT_SUFFIX(nonnull);

struct MuonUnjoinCoercion *unjoin_coercion_allocate(
    mu_inductor_t *inductor, size_t argc)
  MUON_HINT_SUFFIX(malloc);

MuonUnjoinCoercion *unjoin_coercion_activate(
    struct MuonUnjoinCoercion *coercion, MuonType *target)
  MUON_HINT_SUFFIX(nonnull);

struct MuonMeetCoercion *meet_coercion_allocate(
    mu_inductor_t *inductor, size_t argc)
  MUON_HINT_SUFFIX(malloc);

MuonMeetCoercion *meet_coercion_activate(
    struct MuonMeetCoercion *coercion, MuonType *target)
  MUON_HINT_SUFFIX(nonnull);

#endif /* MU_INDUCTOR_COERCION_I */
