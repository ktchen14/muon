#ifndef MU_INDUCTOR_COERCION_I
#define MU_INDUCTOR_COERCION_I

#include <stddef.h>

/// Expands to emit(lower, upper, title, ...) for each kind of coercion
#define MU_EACH_COERCION_KIND(emit, ...) \
  emit(id, ID, Id, ##__VA_ARGS__) \
  emit(simple, SIMPLE, Simple, ##__VA_ARGS__) \
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
} mu_simple_coercion_t;

typedef struct {
  MU_COERCION_HEADER;
  size_t i;
} mu_join_coercion_t;

typedef struct {
  MU_COERCION_HEADER;
  size_t argc;
  const mu_coercion_t *argv[/* argc */];
} mu_unjoin_coercion_t;

void mu_id_coercion_debug(const mu_id_coercion_t *coercion)
  __attribute__((nonnull));

void mu_simple_coercion_debug(const mu_simple_coercion_t *coercion)
  __attribute__((nonnull));

void mu_join_coercion_debug(const mu_join_coercion_t *coercion)
  __attribute__((nonnull));

void mu_unjoin_coercion_debug(const mu_unjoin_coercion_t *coercion)
  __attribute__((nonnull));

#endif /* MU_INDUCTOR_COERCION_I */
