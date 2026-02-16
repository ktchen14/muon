#ifndef MU_INDUCTOR_COERCION_H
#define MU_INDUCTOR_COERCION_H

#include "core.h"
#include "type.h"

#include <stddef.h>

/// Expands to emit(lower, upper, title, ...) for each kind of coercion
#define MU_EACH_COERCION_KIND(emit, ...) \
  emit(id, ID, Id, ##__VA_ARGS__) \
  emit(edge, EDGE, Edge, ##__VA_ARGS__) \
  emit(slot, SLOT, Slot, ##__VA_ARGS__) \
  emit(indirect, INDIRECT, Indirect, ##__VA_ARGS__) \
  emit(variance, VARIANCE, Variance, ##__VA_ARGS__) \
  emit(instance, INSTANCE, Instance, ##__VA_ARGS__) \
  emit(unscheme, UNSCHEME, Unscheme, ##__VA_ARGS__) \
  emit(join, JOIN, Join, ##__VA_ARGS__) \
  emit(unjoin, UNJOIN, Unjoin, ##__VA_ARGS__) \
  emit(meet, MEET, Meet, ##__VA_ARGS__) \
  emit(unmeet, UNMEET, Unmeet, ##__VA_ARGS__)

/// An enumeration over each kind of coercion, e.g. @c MU_ID_COERCION
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_COERCION,
  MU_EACH_COERCION_KIND(MU_EMIT)
#undef MU_EMIT
} MuonCoercionKind;

/// An abstract coercion
typedef const struct MuonCoercion {
  MuonCoercionKind kind;
  const mu_inductor_t *inductor;
  size_t id;
  MuonType *target;
} MuonCoercion;

/// The header that each concrete coercion must have
#define MU_COERCION_HEADER struct MuonCoercion as_coercion

typedef const struct MuonIdCoercion {
  MU_COERCION_HEADER;
} MuonIdCoercion;

typedef const struct MuonEdgeCoercion MuonEdgeCoercion;

typedef const struct MuonIndirectCoercion {
  MU_COERCION_HEADER;
  MuonCoercion *head;
  MuonCoercion *tail;
} MuonIndirectCoercion;

typedef const struct MuonInstanceCoercion {
  MU_COERCION_HEADER;
  const mu_instance_t *instance;
} MuonInstanceCoercion;

typedef const struct MuonVarianceCoercion {
  MU_COERCION_HEADER;
  const mu_core_t *core;
  MuonCoercion *argv[/* target->core->argc */];
} MuonVarianceCoercion;

typedef const struct MuonSlotCoercion {
  MU_COERCION_HEADER;
} MuonSlotCoercion;

/// Coercion of τ to a join type with τ at discriminant @c i
typedef const struct MuonJoinCoercion {
  MU_COERCION_HEADER;
  size_t i;
} MuonJoinCoercion;

/// Coercion of a join type to type τ. Each coercion in argv specifies the
/// coercion to use for that discriminant.
typedef const struct MuonUnjoinCoercion {
  MU_COERCION_HEADER;
  size_t argc;
  MuonCoercion *argv[/* argc */];
} MuonUnjoinCoercion;

/// Coercion of τ to a meet type. Each coercion in argv specifies the coercion
/// of τ to the type at that location.
typedef const struct MuonMeetCoercion {
  MU_COERCION_HEADER;
  size_t argc;
  MuonCoercion *argv[/* argc */];
} MuonMeetCoercion;

/// Coercion of a meet type to type τ, where τ is at index @a i in the meet type
typedef const struct MuonUnmeetCoercion {
  MU_COERCION_HEADER;
  size_t i;
} MuonUnmeetCoercion;

/// Coercion of a scheme type to an instance of its type
typedef const struct MuonUnschemeCoercion {
  MU_COERCION_HEADER;
} MuonUnschemeCoercion;

extern const void *const MU_NO_SUCH_COERCION;

MuonIdCoercion *mu_id_coercion(mu_inductor_t *inductor, MuonType *target)
    /**/ MUON_MALLOC MUON_NONNULL;

MuonSlotCoercion *mu_slot_coercion(mu_inductor_t *inductor, MuonType *target)
    /**/ MUON_MALLOC MUON_NONNULL;

MuonIndirectCoercion *mu_indirect_coercion(
    mu_inductor_t *inductor, MuonCoercion *head, MuonCoercion *tail)
    /**/ MUON_MALLOC MUON_NONNULL;

MuonInstanceCoercion *mu_instance_coercion(
    mu_inductor_t *inductor, MuonType *target, const mu_instance_t *instance)
    /**/ MUON_MALLOC MUON_NONNULL;

MuonVarianceCoercion *mu_variance_coercion(
    mu_inductor_t *inductor,
    MuonType *target,
    const mu_core_t *core,
    MuonCoercion *argv[/* target->core->argc */])
    /**/ MUON_MALLOC MUON_NONNULL_ARGS(1);

MuonJoinCoercion *mu_join_coercion(
    mu_inductor_t *inductor, MuonType *target, size_t i)
    /**/ MUON_MALLOC;

MuonUnjoinCoercion *mu_unjoin_coercion(
    mu_inductor_t *inductor,
    MuonType *target,
    size_t argc,
    MuonCoercion *argv[/* argc */])
    /**/ MUON_MALLOC;

MuonMeetCoercion *mu_meet_coercion(
    mu_inductor_t *inductor,
    MuonType *target,
    size_t argc,
    MuonCoercion *argv[/* argc */])
    /**/ MUON_MALLOC;

MuonUnmeetCoercion *mu_unmeet_coercion(
    mu_inductor_t *inductor, MuonType *target, size_t i)
    /**/ MUON_MALLOC;

MuonUnschemeCoercion *mu_unscheme_coercion(
    mu_inductor_t *inductor, MuonType *target)
    /**/ MUON_MALLOC;

void mu_coercion_debug(MuonCoercion *coercion)
    /**/ MUON_NONNULL;

/// @internal Used to emit each branch in MU_COERCION_ENUMERATOR()
#define MU_COERCION_ENUMERATOR_EMIT(l, upper, title) \
  , Muon##title##Coercion *: MU_##upper##_COERCION

/// Return the enumerator indicative of the @a concrete coercion
#define MU_COERCION_ENUMERATOR(concrete) \
  _Generic((concrete) {0} MU_EACH_COERCION_KIND(MU_COERCION_ENUMERATOR_EMIT))

/// @internal Used to emit each branch in mu_coercion_cast()
#define MU_COERCION_CAST_EMIT(lower, upper, t) \
  , const mu_##lower##_coercion_t *: _kind == MU_##upper##_COERCION

/**
 * @brief Downcast the @a abstract coercion to the <tt>typeof(concrete)</tt>
 *
 * @a abstract should have type <tt>MuonCoercion *</tt>. @a concrete should
 * be, or have, the type of a pointer to a const qualified concrete coercion.
 * Then if @a abstract is an instance of that type, it will be cast to that type
 * and returned. Otherwise, this will return @c NULL.
 *
 * @par Example:
 * @code{.c}
 *   MuonCoercion *abstract_coercion = ...;
 *
 *   mu_variance_coercion_t *coercion;
 *   if ((coercion = mu_coercion_cast(abstract_coercion, coercion)) == NULL)
 *     return ...;
 * @endcode
 *
 * The behavior is undefined if:
 * - @a abstract is @c NULL
 * - @a abstract doesn't have type <tt>MuonCoercion *</tt>
 * - @a concrete isn't, or doesn't have, the type of a const qualified pointer
 *   to a concrete coercion
 */
#define mu_coercion_cast(abstract, concrete) __extension__ ({ \
    MuonCoercion *_abstract = (abstract); \
    typeof(concrete) _concrete; \
    mu_coercion_kind_t _kind = _abstract->kind; \
    int _castable = _Generic(_concrete \
        MU_EACH_COERCION_KIND(MU_COERCION_CAST_EMIT)); \
    _castable ? (typeof(_concrete)) _abstract : NULL; \
  })

#endif /* MU_INDUCTOR_COERCION_H */
