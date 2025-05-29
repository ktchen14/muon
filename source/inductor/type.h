#ifndef MU_INDUCTOR_TYPE_I
#define MU_INDUCTOR_TYPE_I

#include <muon/inductor/type.h>  // IWYU pragma: export

#include "../common.h"
#include "core.h"
#include "induce.h"
#include "universe.h"

#include <assert.h>
#include <limits.h>
#include <stddef.h>

/// @internal An enumeration over each kind of type, e.g. @c _core_type_kind
enum {
#define MU_EMIT(lower, upper, t) _##lower##_type_kind = MU_##upper##_TYPE,
  MU_EACH_TYPE_KIND(MU_EMIT)
#undef MU_EMIT
};

typedef struct {
  MuonType *anterior;
  size_t i : sizeof(size_t) * CHAR_BIT - 1;
  _Bool charge : 1;
} type_cursor_t;

typedef struct {
  MuonType *next;
  type_cursor_t cursor[2];

  // TODO
  union {
    unsigned int status;
    struct {
      _Bool access[2];
      _Bool polymorphic;
    };
  };

  _Alignas(union {
#define MU_EMIT(lower, u, t) mu_##lower##_type_t lower;
    MU_EACH_TYPE_KIND(MU_EMIT)
#undef MU_EMIT
  }) char data[];
} type_header_t;

/// Return the header of the @a type
__attribute__((const, nonnull, returns_nonnull))
static inline type_header_t *type_header(MuonType *type) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wcast-qual"
  return (type_header_t *) ((char *) type - offsetof(type_header_t, data));
#pragma GCC diagnostic pop
}

/// Return the cursor attached to the @a type
__attribute__((const, nonnull, returns_nonnull))
static inline type_cursor_t *type_cursor(MuonType *type, _Bool charge) {
  return &type_header(type)->cursor[charge];
}

static _Thread_local _Bool charge;

/// Continue into the type
static inline MuonType *type_continue(
    MuonType *type, MuonType *next, _Bool next_charge) {
  type_cursor_t *cursor = type_cursor(next, next_charge);
  assert(cursor->anterior == NULL && cursor->i == 0);
  cursor->charge = charge;
  charge = next_charge;
  cursor->anterior = type;
  return next;
}

/// Return from the type
__attribute__((nonnull))
static inline MuonType *type_return(MuonType *type) {
  type_cursor_t *cursor = type_cursor(type, charge);
  charge = cursor->charge;
  MuonType *anterior = cursor->anterior;
  *cursor = (type_cursor_t) {0};
  assert(anterior != NULL || charge == 0);
  return anterior;
}

/// Return the <em>i</em>th type in the abstract @a type
static inline MuonType *type_next(
    MuonType *type, _Bool *next_charge) {
  type_cursor_t *cursor = type_cursor(type, charge);
  *next_charge = charge;

  switch ON_ABSTRACT_OBJECT(type) {
    case IS_KIND_OF(core_type): {
      const mu_core_t *core = core_type->core;

      if (cursor->i >= core->argc)
        return NULL;

      mu_variance_t variance = core->argv[cursor->i].variance;
      assert(variance != MU_INVARIANCE);
      if (variance == MU_CONTRAVARIANCE)
        *next_charge = !*next_charge;
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

mu_core_type_t *core_type_allocate(induce_t *induce, const mu_core_t *core)
  __attribute__((malloc, nonnull));

const mu_core_type_t *core_type_activate(mu_core_type_t *type)
  __attribute__((nonnull, warn_unused_result));

mu_scheme_type_t *scheme_type_allocate(induce_t *induce, size_t argc)
  __attribute__((malloc, nonnull));

const mu_scheme_type_t *scheme_type_activate(
    mu_scheme_type_t *type, MuonType *matter)
  __attribute__((nonnull, warn_unused_result));

mu_join_type_t *join_type_allocate(induce_t *induce, size_t argc)
  __attribute__((malloc, nonnull));

const mu_join_type_t *join_type_activate(mu_join_type_t *join)
  __attribute__((nonnull, warn_unused_result));

void type_debug(MuonType *type, _Bool expand)
  __attribute__((nonnull));

__attribute__((nonnull))
static inline MuonType *assign_solution(
    const mu_variable_type_t *variable_type, MuonType *solution) {
  return ((mu_variable_type_t *) variable_type)->solution = solution;
}

#endif /* MU_INDUCTOR_TYPE_I */
