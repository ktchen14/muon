#ifndef MUON_COMMON_DEBUG_I
#define MUON_COMMON_DEBUG_I

#include "common.h"

#include <stdarg.h>
#include <stdio.h>

/// Whether to emit symbol debugging information
extern _Thread_local _Bool debug_scan;

/// Like <tt>fprintf(muon_debug_stream, format, ...)</tt>
MUON_HINT(format(printf, 1, 2), nonnull(1))
static inline void debug(const char *restrict format, ...) {
  FILE *stream = muon_debug_stream != NULL ? muon_debug_stream : stderr;

  va_list variadic;
  va_start(variadic, format);
  vfprintf(stream, format, variadic);
  va_end(variadic);
}

#endif /* MUON_COMMON_DEBUG_I */
