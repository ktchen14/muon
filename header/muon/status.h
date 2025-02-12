#ifndef MU_STATUS_H
#define MU_STATUS_H

#include <stddef.h>

#include "stator.h"

/**
 * @brief Source location of a Muon syntax object
 */
typedef struct {
  /// Name of the source file or stream as a Muon string
  const char *name;

  /// Byte offset into the source file or stream (zero-indexed)
  size_t offset;

  /// Length, in bytes
  size_t length;

  /// Line number in the source file or stream (one-indexed)
  size_t line;

  /// Column number in the source file or stream (one-indexed)
  size_t column;
} mu_source_t;

typedef struct {
  /// Byte offset into the source file or stream (zero-indexed)
  size_t offset;

  /// Length, in bytes
  size_t length;

  /// Line number in the source file or stream (one-indexed)
  size_t line;

  /// Column number in the source file or stream (one-indexed)
  size_t column;

  /// Length of the name of the source file or stream (not counting the null
  /// terminator), within data
  size_t name_length;

  /// Length of the actual text of the memo (not counting the null terminator),
  /// within data
  size_t text_length;

  char data[/* name + length + 2 */];
} mu_memo_t;

typedef struct mu_status_t mu_status_t;

const mu_memo_t *mu_memo(
    mu_status_t *status,
    const char *restrict format,
    ...)
  __attribute__((format(printf, 2, 3), nonnull));

__attribute__((const, nonnull, returns_nonnull))
static inline const char *mu_memo_name(const mu_memo_t *memo) {
  return memo->data;
}

__attribute__((nonnull, pure, returns_nonnull))
static inline const char *mu_memo_text(const mu_memo_t *memo) {
  return &memo->data[memo->name_length + 1];
}

size_t mu_status_length(const mu_status_t *status)
  __attribute__((nonnull, pure));

#endif /* MU_STATUS_H */
