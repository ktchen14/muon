#ifndef MU_STATOR_TYPE_I
#define MU_STATOR_TYPE_I

#include <muon/stator/type.h>  // IWYU pragma: export

#include "abstract_type.h"     // IWYU pragma: export
#include "boolean_type.h"      // IWYU pragma: export
#include "integer_type.h"      // IWYU pragma: export
#include "lambda_type.h"       // IWYU pragma: export
#include "record_type.h"       // IWYU pragma: export
#include "variable_type.h"     // IWYU pragma: export
#include "vector_type.h"       // IWYU pragma: export

#include "../common.h"
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
  _Alignas(union {
#define MU_EMIT(lower, u, t) mu_##lower##_type_t lower;
    MU_EACH_TYPE_KIND(MU_EMIT)
#undef MU_EMIT
  }) char data[];
} type_header_t;

/// @internal Allocate a type of size @a size in the @a engine
__attribute__((malloc, nonnull))
static inline void *type_allocate(mu_engine_t *engine, size_t size) {
  if (rare((size = struct_size(type_header_t, data, size)) == 0))
    return errno = ENOMEM, NULL;

  type_header_t *header;
  if ((header = engine_allocate(engine, size)) == NULL)
    return NULL;
  *header = (type_header_t) {0};

  return header->data;
}

/// @internal Assign the abstract @a type to the @a engine
__attribute__((nonnull, returns_nonnull))
static inline mu_type_t *assign_type(mu_engine_t *engine, mu_type_t *type) {
  type->as_stator.engine = engine;
  type->as_stator.id = engine->type_number++;
  return type;
}

/// Assign the concrete @a type to the @a engine
#define assign_type(engine, type) \
  ((typeof((type))) (assign_type)((engine), &(type)->as_type))

/// Return the cursor attached to the @a type
__attribute__((const, nonnull, returns_nonnull))
static inline type_cursor_t *type_cursor(const mu_type_t *type) {
  type_header_t *header = (type_header_t *) (
      (char *) type - offsetof(type_header_t, data));
  return &header->cursor;
}

/// Continue into the type
__attribute__((nonnull(2)))
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
static inline const mu_type_t *type_import(
    const mu_type_t *type, import_t *import) {
#define MU_EMIT(lower, upper, _) \
    case MU_##upper##_TYPE: { \
      const mu_##lower##_type_t *result = (const mu_##lower##_type_t *) type; \
      if ((result = lower##_type_import(result, import)) == NULL) \
        return NULL; \
      return &result->as_type; \
    }
  switch (type->kind) { MU_EACH_TYPE_KIND(MU_EMIT) }
#undef MU_EMIT

  __builtin_unreachable();
}

#endif /* MU_STATOR_TYPE_I */
