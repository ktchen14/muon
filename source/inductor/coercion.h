#ifndef MU_INDUCTOR_COERCION_I
#define MU_INDUCTOR_COERCION_I

#include "../common.h"
#include "core.h"
#include "type.h"

#include <stddef.h>

/// Expands to emit(lower, upper, title, ...) for each kind of coercion
#define MU_EACH_COERCION_KIND(emit, ...) \
  emit(id, ID, Id, ##__VA_ARGS__) \
  emit(edge, EDGE, Edge, ##__VA_ARGS__) \
  emit(indirect, INDIRECT, Indirect, ##__VA_ARGS__) \
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
typedef struct mu_coercion_t mu_coercion_t;
struct mu_coercion_t {
  mu_coercion_kind_t kind;
  const mu_type_t *target;
};

/// The header that each concrete coercion must have
#define MU_COERCION_HEADER mu_coercion_t as_coercion

typedef mu_coercion_t mu_id_coercion_t;

typedef struct {
  MU_COERCION_HEADER;
  const mu_type_t *target;
  const mu_type_t *source;
} mu_edge_coercion_t;

typedef struct {
  MU_COERCION_HEADER;
  const mu_coercion_t *head;
  const mu_coercion_t *tail;
} mu_indirect_coercion_t;

typedef struct {
  MU_COERCION_HEADER;
  const mu_core_type_t *target;
  const mu_coercion_t *argv[/* target->core->argc */];
} mu_variance_coercion_t;

typedef struct {
  MU_COERCION_HEADER;
  const mu_type_t *target;
  const record_instance_t *instance;
  const mu_coercion_t *argv[/* instance->target->argc */];
} mu_record_coercion_t;

/// Coercion of τ to a join type with τ at discriminant @c i
typedef struct {
  MU_COERCION_HEADER;
  const mu_variable_type_t *target;
  size_t i;
} mu_join_coercion_t;

/// Coercion of a join type to type τ. Each coercion in argv specifies the
/// coercion to use for that discriminant.
typedef struct {
  MU_COERCION_HEADER;
  const mu_type_t *target;
  size_t argc;
  const mu_coercion_t *argv[/* argc */];
} mu_unjoin_coercion_t;

extern const void *const NO_SUCH_COERCION;

/// @internal An enumeration over each kind of coercion, e.g. @c _id_coercion_kind
enum {
#define MU_EMIT(lower, u, t) _##lower##_coercion_kind,
  MU_EACH_COERCION_KIND(MU_EMIT)
#undef MU_EMIT
};

const mu_edge_coercion_t *mu_edge_coercion(
    const mu_type_t *source, const mu_type_t *target)
  __attribute__((malloc, nonnull));

const mu_indirect_coercion_t *mu_indirect_coercion(
    const mu_coercion_t *head, const mu_coercion_t *tail)
  __attribute__((malloc, nonnull));

const mu_variance_coercion_t *mu_variance_coercion(
    const mu_core_type_t *target, const mu_coercion_t *argv[/* target->core->argc */])
  __attribute__((malloc, nonnull(1)));

const mu_record_coercion_t *mu_record_coercion(const record_instance_t *instance)
  __attribute__((malloc, nonnull));

const mu_join_coercion_t *mu_join_coercion(
    const mu_variable_type_t *target, size_t i)
  __attribute__((malloc));

const mu_unjoin_coercion_t *mu_unjoin_coercion(
    const mu_type_t *target, size_t argc, const mu_coercion_t *argv[/* argc */])
  __attribute__((malloc, nonnull));

mu_variance_coercion_t *variance_coercion_allocate(const mu_core_type_t *target)
  __attribute__((malloc));

const mu_variance_coercion_t *variance_coercion_activate(
    mu_variance_coercion_t *coercion)
  __attribute__((nonnull));

mu_record_coercion_t *record_coercion_allocate(const record_instance_t *instance)
  __attribute__((malloc));

const mu_record_coercion_t *record_coercion_activate(
    mu_record_coercion_t *coercion)
  __attribute__((nonnull));

mu_unjoin_coercion_t *unjoin_coercion_allocate(size_t argc)
  __attribute__((malloc));

const mu_unjoin_coercion_t *unjoin_coercion_activate(
    mu_unjoin_coercion_t *coercion, const mu_type_t *target)
  __attribute__((nonnull));

__attribute__((nonnull, pure))
static inline const mu_type_t *mu_coercion_target(
    const mu_coercion_t *coercion, const mu_type_t *source) {
  switch ON_ABSTRACT_OBJECT(coercion) {
    case MU_ID_COERCION:
      return source;

    case IS_KIND_OF(edge_coercion):
      return edge_coercion->target;

    case IS_KIND_OF(indirect_coercion):
      return mu_coercion_target(indirect_coercion->tail, source);

    case IS_KIND_OF(variance_coercion):
      return &variance_coercion->target->as_type;

    case IS_KIND_OF(record_coercion):
      return record_coercion->target;

    case IS_KIND_OF(join_coercion):
      return &join_coercion->target->as_type;

    case IS_KIND_OF(unjoin_coercion):
      return unjoin_coercion->target;
  }
  __builtin_unreachable();
}

void mu_coercion_debug(const mu_coercion_t *coercion)
  __attribute__((nonnull));

/// @internal Used to emit each branch in mu_coercion_cast()
#define MU_COERCION_CAST_EMIT(lower, upper, t) \
  , const mu_##lower##_coercion_t *: _kind == MU_##upper##_COERCION

/**
 * @brief Downcast the @a abstract coercion to the <tt>typeof(concrete)</tt>
 *
 * @a abstract should have type <tt>const mu_coercion_t *</tt>. @a concrete should
 * be, or have, the type of a pointer to a const qualified concrete coercion. Then
 * if @a abstract is an instance of that type, it will be cast to that type and
 * returned. Otherwise, this will return @c NULL.
 *
 * @par Example:
 * @code{.c}
 *   mu_coercion_t *abstract_coercion = ...;
 *
 *   mu_variance_coercion_t *coercion;
 *   if ((coercion = mu_coercion_cast(abstract_coercion, coercion)) == NULL)
 *     return ...;
 * @endcode
 *
 * The behavior is undefined if:
 * - @a abstract is @c NULL
 * - @a abstract doesn't have type <tt>const mu_coercion_t *</tt>
 * - @a concrete isn't, or doesn't have, the type of a const qualified pointer
 *   to a concrete coercion
 */
#define mu_coercion_cast(abstract, concrete) __extension__ ({ \
    const mu_coercion_t *_abstract = (abstract); \
    typeof(concrete) _concrete; \
    mu_coercion_kind_t _kind = _abstract->kind; \
    int _castable = _Generic(_concrete \
        MU_EACH_COERCION_KIND(MU_COERCION_CAST_EMIT)); \
    _castable ? (typeof(_concrete)) _abstract : NULL; \
  })

#endif /* MU_INDUCTOR_COERCION_I */
