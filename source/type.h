#ifndef MU_TYPE_I
#define MU_TYPE_I

#include <muon/type.h>      // IWYU pragma: export

#include "type/integer.h"   // IWYU pragma: export
#include "type/variable.h"  // IWYU pragma: export
#include "type/vector.h"    // IWYU pragma: export

#include "engine.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>

typedef struct {
  const mu_type_t *anterior;
  size_t i;
} type_cursor_t;

typedef struct {
  type_cursor_t cursor;
  _Alignas(max_align_t) char data[];
} type_header_t;

/// Return the cursor in the @a type
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

__attribute__((malloc, nonnull))
static inline void *type_allocate(mu_engine_t *engine, size_t size) {
  if (rare((size = struct_size(type_header_t, data, size)) == 0))
    return errno = ENOMEM, NULL;

  type_header_t *header;
  if (rare((header = engine_allocate(engine, size)) == NULL))
    return NULL;
  *header = (type_header_t) {0};

  return header->data;
}

__attribute__((nonnull, pure))
static inline const mu_type_t *type_reduce(
    const mu_type_t *type,
    mu_engine_t *engine,
    inductor_t *inductor) {
#define MU_EMIT(lower, upper, _) \
    case MU_##upper##_TYPE: \
      return &lower##_type_reduce((const mu_##lower##_type_t *) type, engine, inductor)->as_type;
  switch (type->kind) { MU_EACH_TYPE_KIND(MU_EMIT) }
#undef MU_EMIT

  __builtin_unreachable();
}

#endif /* MU_TYPE_I */
