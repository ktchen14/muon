#ifndef MU_INDUCTOR_TYPE_I
#define MU_INDUCTOR_TYPE_I

#include <muon/inductor/type.h>  // IWYU pragma: export

#include "core.h"

#include <assert.h>
#include <stddef.h>

typedef struct induce_t induce_t;

typedef struct mu_scheme_t mu_scheme_t;
struct mu_scheme_t {
  const induce_t *induce;
  mu_scheme_t *parent;

  // The lowest id that a type that's a part of this scheme will have
  size_t id;
};

mu_scheme_t *mu_scheme(mu_scheme_t *parent)
  __attribute__((malloc));

mu_core_type_t *core_type_allocate(induce_t *induce, const mu_core_t *core)
  __attribute__((malloc, nonnull));

const mu_core_type_t *core_type_activate(mu_core_type_t *type)
  __attribute__((nonnull));

mu_scheme_type_t *scheme_type_allocate(induce_t *induce, size_t argc)
  __attribute__((malloc, nonnull));

const mu_scheme_type_t *scheme_type_activate(
    mu_scheme_type_t *type, const mu_type_t *matter)
  __attribute__((nonnull));

mu_join_type_t *join_type_allocate(induce_t *induce, size_t argc)
  __attribute__((malloc, nonnull));

const mu_join_type_t *join_type_activate(mu_join_type_t *join)
  __attribute__((nonnull));

void type_debug(const mu_type_t *type, _Bool expand)
  __attribute__((nonnull));

void debug_variable_type_name(const mu_variable_type_t *type);

__attribute__((nonnull))
static inline const mu_type_t *assign_solution(
    const mu_variable_type_t *variable_type, const mu_type_t *solution) {
  return ((mu_variable_type_t *) variable_type)->solution = solution;
}

/// @internal An enumeration over each kind of type, e.g. @c _core_type_kind
enum {
#define MU_EMIT(lower, upper, t) _##lower##_type_kind = MU_##upper##_TYPE,
  MU_EACH_TYPE_KIND(MU_EMIT)
#undef MU_EMIT
};

extern _Thread_local _Bool charge;
extern _Thread_local _Bool next_charge;

typedef struct {
  const mu_type_t *anterior;
  size_t i;
} type_cursor_t;

typedef struct {
  type_cursor_t cursor[2];
  _Alignas(union {
#define MU_EMIT(lower, u, t) mu_##lower##_type_t lower;
    MU_EACH_TYPE_KIND(MU_EMIT)
#undef MU_EMIT
  }) char data[];
} type_header_t;

/// Return the cursor attached to the @a type
__attribute__((const, nonnull, returns_nonnull))
static inline type_cursor_t *type_cursor(const mu_type_t *type) {
  type_header_t *header = (type_header_t *) (
      (char *) type - offsetof(type_header_t, data));
  return &header->cursor[charge];
}

/// Continue into the type
static inline const mu_type_t *type_continue(
    const mu_type_t *type, const mu_type_t *next) {
  type_cursor_t *cursor = type_cursor(next);
  assert(cursor->anterior == NULL && cursor->i == 0);
  charge = next_charge;
  cursor->anterior = type;
  return next;
}

/// Return from the type
__attribute__((nonnull))
static inline const mu_type_t *type_return(const mu_type_t *type) {
  type_cursor_t *cursor = type_cursor(type);
  const mu_type_t *anterior = cursor->anterior;
  *cursor = (type_cursor_t) {0};

  const mu_core_type_t *core_type;
  if (anterior != NULL && (core_type = mu_type_cast(anterior, core_type)) != NULL) {
    const mu_core_t *core = core_type->core;
    cursor = type_cursor(anterior);
    if (core->argv[cursor->i - 1].variance == MU_CONTRAVARIANCE)
      charge = !charge;
  }
  return anterior;
}

/// Return the <em>i</em>th type in the abstract @a type
const mu_type_t *type_next(const mu_type_t *type);

#endif /* MU_INDUCTOR_TYPE_I */
