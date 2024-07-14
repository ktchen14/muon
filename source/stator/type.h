#ifndef MU_STATOR_TYPE_I
#define MU_STATOR_TYPE_I

#include <muon/stator/type.h>  // IWYU pragma: export

#include "abstract_type.h"     // IWYU pragma: export
#include "boolean_type.h"      // IWYU pragma: export
#include "integer_type.h"      // IWYU pragma: export
#include "variable_type.h"     // IWYU pragma: export
#include "vector_type.h"       // IWYU pragma: export

#include <assert.h>
#include <stddef.h>

typedef struct {
  const mu_type_t *anterior;
  size_t i;
} type_cursor_t;

typedef struct {
  type_cursor_t cursor;
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
  return &header->cursor;
}

/// Continue into the type
static inline const mu_type_t *type_continue(
    const mu_type_t *type, const mu_type_t *next) {
  type_cursor_t *cursor = type_cursor(next);
  assert(cursor->anterior == NULL && cursor->i == 0);
  cursor->anterior = type;
  return next;
}

/// Return from the type
__attribute__((nonnull))
static inline const mu_type_t *type_return(const mu_type_t *type) {
  type_cursor_t *cursor = type_cursor(type);
  const mu_type_t *anterior = cursor->anterior;
  *cursor = (type_cursor_t) {0};
  return anterior;
}

__attribute__((nonnull, pure))
static inline const mu_type_t *type_at(const mu_type_t *type, size_t i) {
#define MU_EMIT(lower, upper, _) \
    case MU_##upper##_TYPE: \
      return lower##_type_at((const mu_##lower##_type_t *) type, i);
  switch (type->kind) { MU_EACH_TYPE_KIND(MU_EMIT) }
#undef MU_EMIT

  __builtin_unreachable();
}

__attribute__((nonnull))
static inline const mu_type_t *type_reduce(
    const mu_type_t *type, inductor_t *inductor) {
#define MU_EMIT(lower, upper, _) \
    case MU_##upper##_TYPE: \
      return &lower##_type_reduce((const mu_##lower##_type_t *) type, inductor)->as_type;
  switch (type->kind) { MU_EACH_TYPE_KIND(MU_EMIT) }
#undef MU_EMIT

  __builtin_unreachable();
}

#endif /* MU_STATOR_TYPE_I */
