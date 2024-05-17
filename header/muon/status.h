#ifndef MU_STATUS_H
#define MU_STATUS_H

#include <stddef.h>

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
  /// Length of the name of the source file or stream (not counting the null
  /// terminator), within data
  size_t name_length;

  /// Length of the actual text of the memo (not counting the null terminator),
  /// within data
  size_t text_length;

  /// Byte offset into the source file or stream (zero-indexed)
  size_t offset;

  /// Length, in bytes
  size_t length;

  /// Line number in the source file or stream (one-indexed)
  size_t line;

  /// Column number in the source file or stream (one-indexed)
  size_t column;

  char data[/* name + length + 2 */];
} mu_memo_t;

#endif /* MU_STATUS_H */
