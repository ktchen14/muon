#ifndef MUON_COMMON_DEBUG_I
#define MUON_COMMON_DEBUG_I

#include "common.h"

#include <stdarg.h>
#include <stdio.h>

/// Literal printf specifier for a kind
#define PRIsKIND "%s%s%s"

/// Used with PRIsKIND to emit the @a text as a core kind
#define DEBUG_CORE_KIND(text) "", (text), ""

/// Used with PRIsKIND to emit the @a text as a coercion kind
#define DEBUG_COERCION_KIND(text) \
  muon_debug_colorize ? "\x1b[0;34m" : "", (text), muon_debug_colorize ? "\x1b[0m" : ""

/// Literal printf specifier for a name
#define PRIsNAME "%s"

/// Used with PRIsNAME to emit the text of the @a name
#define DEBUG_NAME(name) ((name)->text)

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
