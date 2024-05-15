#ifndef MU_STATUS_H
#define MU_STATUS_H

#include <stddef.h>

/**
 * @brief Source location of a Muon syntax object
 */
typedef struct {
  /// Name of the source file or stream
  const char *name;

  /// Byte offset into the source file or stream
  size_t offset;

  /// Length (in bytes)
  size_t length;

  /// Line number in the source file or stream
  size_t line;

  /// Column number in the source file or stream
  size_t column;
} mu_source_t;

#endif /* MU_STATUS_H */
