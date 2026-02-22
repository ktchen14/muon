#ifndef MU_INDUCTOR_TYPE_I
#define MU_INDUCTOR_TYPE_I

#include <muon/inductor/type.h> // IWYU pragma: export

#include "../common.h"
#include "core.h"
#include "induce.h"
#include "universe.h"

#include <assert.h>
#include <limits.h>
#include <stddef.h>

#define INTERNAL_IS_CONCRETE_TYPE(type, name) \
  MU_TYPE_ENUMERATOR(type):; __typeof__(type) name = _object;

#define IS_CONCRETE_TYPE(...) INTERNAL_IS_CONCRETE_TYPE(__VA_ARGS__)

#define nominate(name) , name

typedef struct {
  /// @internal The type to return to, or @c NULL if this is the root type
  MuonType *anterior;

  size_t i : sizeof(size_t) * CHAR_BIT - 1;

  /// @internal Used to decide which cursor to return to in the @a anterior type
  size_t charge : 1;

  /// @internal Used to mark if the type is accessible
  size_t access : 1;
} TypeCursor;

typedef struct {
  MuonType *next;
  TypeCursor cursor[2];

  // TODO
  union {
    unsigned int status;
    struct {
      _Bool access[2];
      _Bool polymorphic;
    };
  };

  _Alignas(union {
#define MU_EMIT(lower, u, title) Muon##title##Type lower;
    MUON_EACH_TYPE_KIND(MU_EMIT)
#undef MU_EMIT
  }) char data[];
} TypeHeader;

/// Return the header of the @a type
MUON_HINT(const, nonnull, returns_nonnull)
static inline TypeHeader *type_header(MuonType *type) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wcast-qual"
  return (TypeHeader *) ((char *) type - offsetof(TypeHeader, data));
#pragma GCC diagnostic pop
}

/// Return the cursor attached to the @a type
MUON_HINT(const, nonnull, returns_nonnull)
static inline TypeCursor *type_cursor(MuonType *type, _Bool charge) {
  return &type_header(type)->cursor[charge];
}

static _Thread_local _Bool charge;

/// Continue into the type
static inline MuonType *type_continue(
    MuonType *type, MuonType *next, _Bool next_charge) {
  TypeCursor *cursor = type_cursor(next, next_charge);
  assert(cursor->anterior == NULL && cursor->i == 0);
  cursor->charge = charge;
  charge = next_charge;
  cursor->anterior = type;
  return next;
}

/// Return from the type
MUON_HINT(nonnull)
static inline MuonType *type_return(MuonType *type) {
  TypeCursor *cursor = type_cursor(type, charge);
  charge = cursor->charge;
  MuonType *anterior = cursor->anterior;
  *cursor = (TypeCursor) {0};
  assert(anterior != NULL || charge == 0);
  return anterior;
}

/// Return the <em>i</em>th type in the abstract @a type
static inline MuonType *type_next(MuonType *type, _Bool *next_charge) {
  TypeCursor *cursor = type_cursor(type, charge);
  *next_charge = charge;

  switch ON_ABSTRACT_OBJECT(type) {
    case IS_CONCRETE_TYPE(MuonCoreType * nominate(core_type)) {
      const MuonCore *core = core_type->core;

      if (cursor->i >= core->argc)
        return NULL;

      MuonVariance variance = core->argv[cursor->i].variance;
      assert(variance != MUON_INVARIANCE);
      if (variance == MUON_CONTRAVARIANCE)
        *next_charge = !*next_charge;
      return core_type->argv[cursor->i++];
    }

    case IS_CONCRETE_TYPE(MuonSchemeType * nominate(scheme_type))
      if (cursor->i > 0)
        return NULL;
      return cursor->i++, scheme_type->matter;

    case IS_CONCRETE_TYPE(MuonJoinType * nominate(join_type))
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

struct MuonCoreType *core_type_allocate(induce_t *induce, const MuonCore *core)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonCoreType *core_type_activate(struct MuonCoreType *type)
  MUON_HINT_SUFFIX(nonnull, warn_unused_result);

struct MuonSchemeType *scheme_type_allocate(induce_t *induce, size_t argc)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonSchemeType *scheme_type_activate(
    struct MuonSchemeType *type, MuonType *matter)
  MUON_HINT_SUFFIX(nonnull, warn_unused_result);

struct MuonJoinType *join_type_allocate(induce_t *induce, size_t argc)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonJoinType *join_type_activate(struct MuonJoinType *join)
  MUON_HINT_SUFFIX(nonnull, warn_unused_result);

void type_debug(MuonType *type, _Bool expand)
  MUON_HINT_SUFFIX(nonnull);

MUON_HINT(nonnull)
static inline MuonType *assign_solution(
    MuonVariableType *variable_type, MuonType *solution) {
  return ((struct MuonVariableType *) variable_type)->solution = solution;
}

#endif /* MU_INDUCTOR_TYPE_I */
