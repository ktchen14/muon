#ifndef MU_INDUCTOR_TYPE_I
#define MU_INDUCTOR_TYPE_I

#include <muon/inductor/type.h>  // IWYU pragma: export

#include "../common.h"
#include "core.h"
#include "induce.h"
#include "universe.h"

#include <assert.h>
#include <stddef.h>

typedef struct induce_t induce_t;

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

static _Thread_local _Bool charge;
static _Thread_local _Bool next_charge;

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

  if (anterior == NULL)
    return NULL;

  const mu_core_type_t *core_type;
  if ((core_type = mu_type_cast(anterior, core_type)) != NULL) {
    const mu_core_t *core = core_type->core;
    cursor = type_cursor(anterior);
    if (core->argv[cursor->i - 1].variance == MU_CONTRAVARIANCE)
      charge = !charge;
  }
  return anterior;
}

/// Return the <em>i</em>th type in the abstract @a type
static inline const mu_type_t *type_next(const mu_type_t *type) {
  type_cursor_t *cursor = type_cursor(type);
  next_charge = charge;

  switch ON_ABSTRACT_OBJECT(type) {
    case IS_KIND_OF(core_type): {
      const mu_core_t *core = core_type->core;

      if (cursor->i >= core->argc)
        return NULL;

      mu_variance_t variance = core->argv[cursor->i].variance;
      assert(variance != MU_INVARIANCE);
      if (variance == MU_CONTRAVARIANCE)
        next_charge = !next_charge;
      return core_type->argv[cursor->i++];
    }

    case IS_KIND_OF(scheme_type):
      if (cursor->i > 0)
        return NULL;
      return cursor->i++, scheme_type->matter;

    case IS_KIND_OF(join_type):
      assert(charge == 0);
      return cursor->i < join_type->argc ? join_type->argv[cursor->i++] : NULL;

    case MU_VARIABLE_TYPE: {
      const universe_t *universe = &type->induce->universe;

      for (size_t i; (i = cursor->i++) < universe->length;) {
        const type_edge_t *edge = &universe->data[i];
        if (edge->vertex[!charge] == type)
          return edge->vertex[charge];
      }
      return NULL; 
    }
  }
  __builtin_unreachable();
}

#endif /* MU_INDUCTOR_TYPE_I */
