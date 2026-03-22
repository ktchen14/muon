#ifndef MUON_ENGINE_TYPE_I
#define MUON_ENGINE_TYPE_I

#include <muon/engine/type.h>

#include "common.h"
#include "core.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

/// A type and charge
typedef struct {
  MuonType *type; _Bool charge; //-
} Attitude;

typedef uintptr_t AttitudeCode;

[[gnu::const]] static inline AttitudeCode attitude_encode(Attitude attitude) {
  assert(((AttitudeCode) attitude.type & 1) == 0);
  return (AttitudeCode) attitude.type | attitude.charge;
}

[[gnu::const]] static inline Attitude attitude_decode(AttitudeCode attitude) {
  return (Attitude) {(MuonType *) (attitude & ~(uintptr_t) 1), attitude & 1};
}

/// Return whether attitude @a a is equivalent to attitude @a b
[[gnu::const]] static inline _Bool attitude_eq(Attitude a, Attitude b) {
  return a.type == b.type && a.charge == b.charge;
}

/// Return whether the @a attitude is null
[[gnu::const]] static inline _Bool attitude_isnull(Attitude attitude) {
  return attitude_eq(attitude, (Attitude) {});
}

/// Return @a attitude with the same type and opposite charge
[[gnu::const]] static inline Attitude attitude_invert(Attitude attitude) {
  return (Attitude) {attitude.type, !attitude.charge};
}

typedef struct {
  /// @internal Used to traverse a type tree
  struct TypeCursor {
    AttitudeCode anterior;
    size_t i;
  } cursor[2];

  _Alignas(union {
#define MUON_EMIT(Title, lower, U) Muon##Title lower;
    MUON_EACH_TYPE(MUON_EMIT)
#undef MUON_EMIT
  }) char type[];
} TypeHeader;

[[gnu::const, gnu::nonnull, gnu::returns_nonnull]]
static inline TypeHeader *type_header(MuonType *type) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wcast-qual"
  return (TypeHeader *) ((char *) type - offsetof(TypeHeader, type));
#pragma GCC diagnostic pop
}

/// Return the cursor of the @a attitude
[[gnu::const, gnu::returns_nonnull]]
static inline struct TypeCursor *type_cursor(Attitude attitude) {
  return &type_header(attitude.type)->cursor[attitude.charge];
}

/// Continue into the @a attitude
static inline Attitude type_continue(Attitude origin, Attitude next) {
  struct TypeCursor *cursor = type_cursor(next);
  Attitude attitude = attitude_decode(cursor->anterior);
  assert(attitude_isnull(attitude) && cursor->i == 0);
  return cursor->anterior = attitude_encode(origin), next;
}

/// Return from the @a attitude
static inline Attitude type_return(Attitude origin) {
  struct TypeCursor *cursor = type_cursor(origin);
  origin = attitude_decode(cursor->anterior);
  return *cursor = (struct TypeCursor) {}, origin;
}

/// Return whether the @a type is an implicit type
[[gnu::nonnull, gnu::pure]]
static inline _Bool is_implicit_type(MuonType *type) {
  return type->tag == MUON_IMPLICIT_TYPE;
}

/// @internal Used in ON_ABSTRACT_TYPE()
static _Thread_local const void *abstract_type;

/// Used with IS_CONCRETE_TYPE() to switch on the tag of the abstract @a type
#define ON_ABSTRACT_TYPE(type) ((typeof(type)) {abstract_type = (type)}->tag)

/// Emit a case within a switch ON_ABSTRACT_TYPE()
#define IS_CONCRETE_TYPE(...) \
  MUON_TYPE_TAG(typeof((struct { __VA_ARGS__, *_; }) {}._)): \
    __VA_ARGS__ = abstract_type;

/// Return the <em>i</em>th type in the abstract type @a origin
static inline Attitude type_at(Attitude origin, size_t i) {
  switch ON_ABSTRACT_TYPE(origin.type) {
    case IS_CONCRETE_TYPE(MuonCoreType *core_type)
      if (i < core_argc(core_type->core)) {
        MuonCoreMember member = core_at(core_type->core, i);
        _Bool charge = origin.charge ^ member.variance;
        return (Attitude) {core_type->argv[member.i], charge};
      }
      return (Attitude) {};

    case MUON_IMPLICIT_TYPE:
      return (Attitude) {};

    case IS_CONCRETE_TYPE(MuonJoinType *join_type)
      if (origin.charge == 0 && i < join_type->argc)
        return (Attitude) {join_type->argv[i], origin.charge};
      return (Attitude) {};

    case IS_CONCRETE_TYPE(MuonMeetType *meet_type)
      if (origin.charge == 1 && i < meet_type->argc)
        return (Attitude) {meet_type->argv[i], origin.charge};
      return (Attitude) {};

    case IS_CONCRETE_TYPE(MuonSchemeType *scheme_type)
      return (Attitude) {
        (MuonType *[]) {scheme_type->matter, NULL}[i], origin.charge
      };

    case IS_CONCRETE_TYPE(MuonVariableType *variable_type)
      MuonType *next = (MuonType *[]) {
        variable_type->join, variable_type->meet
      }[origin.charge];
      return (Attitude) {(MuonType *[]) {next, NULL}[i], origin.charge};
  }
}

struct MuonCoreType *core_type_allocate(MuonEngine *engine, MuonCore *core)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonCoreType *core_type_activate(struct MuonCoreType *type)
  MUON_HINT_SUFFIX(nonnull);

struct MuonJoinType *join_type_allocate(MuonEngine *engine, size_t argc)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonJoinType *join_type_activate(struct MuonJoinType *join)
  MUON_HINT_SUFFIX(nonnull);

struct MuonMeetType *meet_type_allocate(MuonEngine *engine, size_t argc)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonMeetType *meet_type_activate(struct MuonMeetType *meet)
  MUON_HINT_SUFFIX(nonnull);

struct MuonSchemeType *scheme_type_allocate(MuonEngine *engine)
  MUON_HINT_SUFFIX(malloc, nonnull);

MuonSchemeType *scheme_type_activate(
    struct MuonSchemeType *type, MuonType *matter)
  MUON_HINT_SUFFIX(nonnull);

#endif /* MUON_ENGINE_TYPE_I */
