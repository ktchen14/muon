#ifndef MU_STATOR_TEST_I
#define MU_STATOR_TEST_I

#include <muon/stator/test.h>  // IWYU pragma: export

#include "abstract_test.h"     // IWYU pragma: export
#include "member_test.h"       // IWYU pragma: export

#include "engine.h"
#include "../common.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>

typedef struct {
  const mu_test_t *anterior;
  size_t i;
} test_cursor_t;

typedef struct {
  test_cursor_t cursor;
  _Alignas(union {
#define MU_EMIT(lower, u, t) mu_##lower##_test_t lower;
    MU_EACH_TEST_KIND(MU_EMIT)
#undef MU_EMIT
  }) char data[];
} test_header_t;

/// @internal Allocate a test of size @a size in the @a engine
__attribute__((malloc, nonnull))
static inline void *test_allocate(mu_engine_t *engine, size_t size) {
  if (rare((size = struct_size(test_header_t, data, size)) == 0))
    return errno = ENOMEM, NULL;

  test_header_t *header;
  if ((header = engine_allocate(engine, size)) == NULL)
    return NULL;
  *header = (test_header_t) {0};

  return header->data;
}

/// @internal Assign the abstract @a test to the @a engine
__attribute__((nonnull, returns_nonnull))
static inline mu_test_t *assign_test(mu_engine_t *engine, mu_test_t *test) {
  test->as_stator.engine = engine;
  test->as_stator.id = engine->test_number++;
  return test;
}

/// Assign the concrete @a test to the @a engine
#define assign_test(engine, test) \
  ((typeof((test))) (assign_test)((engine), &(test)->as_test))

/// Return the cursor attached to the @a test
__attribute__((const, nonnull, returns_nonnull))
static inline test_cursor_t *test_cursor(const mu_test_t *test) {
  test_header_t *header = (test_header_t *) (
      (char *) test - offsetof(test_header_t, data));
  return &header->cursor;
}

/// Continue into the test
static inline const mu_test_t *test_continue(
    const mu_test_t *test, const mu_test_t *next) {
  test_cursor_t *cursor = test_cursor(next);
  assert(cursor->anterior == NULL && cursor->i == 0);
  cursor->anterior = test;
  return next;
}

/// Return from the test
__attribute__((nonnull))
static inline const mu_test_t *test_return(const mu_test_t *test) {
  test_cursor_t *cursor = test_cursor(test);
  const mu_test_t *anterior = cursor->anterior;
  *cursor = (test_cursor_t) {0};
  return anterior;
}

__attribute__((nonnull, pure))
static inline const mu_test_t *test_at(const mu_test_t *test, size_t i) {
#define MU_EMIT(lower, upper, _) \
    case MU_##upper##_TEST: \
      return lower##_test_at((const mu_##lower##_test_t *) test, i);
  switch (test->kind) { MU_EACH_TEST_KIND(MU_EMIT) }
#undef MU_EMIT

  __builtin_unreachable();
}

#endif /* MU_STATOR_TEST_I */
