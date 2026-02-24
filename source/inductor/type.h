#ifndef MUON_INDUCTOR_TYPE_I
#define MUON_INDUCTOR_TYPE_I

#include <muon/inductor/type.h> // IWYU pragma: export

#include "../common.h"
#include "core.h"
#include "induce.h"
#include "universe.h"

#include <assert.h>
#include <limits.h>
#include <stddef.h>

/// Emit a case within a switch ON_ABSTRACT_OBJECT()
#define IS_CONCRETE_TYPE(...) \
  MUON_TYPE_TAG(typeof(&(union { __VA_ARGS__, _; }) {}._)): \
    __VA_ARGS__ = _object;

typedef struct {
  MuonType *next;

  union {
    /// @internal Used to traverse a type tree
    struct TypeCursor {
      MuonType *type;
      size_t i : sizeof(size_t) * CHAR_BIT - 2;

      /// @internal Used to decide which cursor to return to in the @a anterior type
      size_t charge : 1;

      /// @internal Used to mark if the type is accessible
      size_t access : 1;
    } cursor[2];

    /// @internal Used to assemble a type list
    struct TypeSeries {
      MuonType *next; size_t n; //-
    } series[2];

    static_assert(sizeof(struct TypeCursor) == sizeof(struct TypeSeries));
  };

  // TODO
  union {
    unsigned int status;
    struct {
      _Bool access[2];
      _Bool polymorphic;
    };
  };

  _Alignas(union {
#define MUON_EMIT(Title, lower, U) Muon##Title lower;
    MUON_EACH_TYPE(MUON_EMIT)
#undef MUON_EMIT
  }) struct MuonType type[];
} TypeHeader;

/// Return the header of the @a type
MUON_HINT(const, nonnull, returns_nonnull)
static inline TypeHeader *type_header(MuonType *type) {
  const size_t offset = offsetof(TypeHeader, type);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wcast-qual"
  return (TypeHeader *) ((char *) type - offset);
#pragma GCC diagnostic pop
}

/// Return the @a charge cursor of the @a type
MUON_HINT(const, nonnull, returns_nonnull)
static inline struct TypeCursor *type_cursor(MuonType *type, _Bool charge) {
  return &type_header(type)->cursor[charge];
}

/// Return the @a charge series of the @a type
MUON_HINT(const, nonnull, returns_nonnull)
static inline struct TypeSeries *type_series(MuonType *type, _Bool charge) {
  const size_t offset = offsetof(TypeHeader, type);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wcast-qual"
  TypeHeader *header = (TypeHeader *) ((char *) type - offset);
#pragma GCC diagnostic pop
  return &header->series[charge];
}

static _Thread_local _Bool charge;

/// Continue into the type
static inline MuonType *type_continue(
    MuonType *type, MuonType *next, _Bool next_charge) {
  struct TypeCursor *cursor = type_cursor(next, next_charge);
  assert(cursor->type == NULL && cursor->i == 0);
  cursor->charge = charge;
  charge = next_charge;
  cursor->type = type;
  return next;
}

/// Return from the type
MUON_HINT(nonnull)
static inline MuonType *type_return(MuonType *type) {
  struct TypeCursor *cursor = type_cursor(type, charge);
  charge = cursor->charge;
  MuonType *anterior = cursor->type;
  *cursor = (struct TypeCursor) {0};
  return anterior;
}

/// Return the <em>i</em>th type in the abstract @a type
static inline MuonType *type_next(MuonType *type, _Bool *next_charge) {
  struct TypeCursor *cursor = type_cursor(type, charge);
  *next_charge = charge;

  switch ON_ABSTRACT_OBJECT(type) {
    case IS_CONCRETE_TYPE(MuonCoreType *core_type) {
      MuonCore *core = core_type->core;

      size_t i;
      while ((i = cursor->i++) >> 1 < core->argc) {
        MuonCoreMember member = core->argv[i >> 1];
        _Bool variance = i & 1;
        if ((member.variance & (1 << variance)) == 0)
          continue;
        *next_charge ^= variance;
        return core_type->argv[i >> 1];
      }
      return NULL;
    }

    case IS_CONCRETE_TYPE(MuonSchemeType *scheme_type)
      if (cursor->i > 0)
        return NULL;
      return cursor->i++, scheme_type->matter;

    case IS_CONCRETE_TYPE(MuonJoinType *join_type)
      assert(charge == 0);
      return cursor->i < join_type->argc ? join_type->argv[cursor->i++] : NULL;

    case MUON_VARIABLE_TYPE: {
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

struct MuonCoreType *core_type_allocate(induce_t *induce, MuonCore *core)
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

#endif /* MUON_INDUCTOR_TYPE_I */
