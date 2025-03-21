#ifndef MU_STATOR_DEBUG_I
#define MU_STATOR_DEBUG_I

#include <stdio.h>

/// Whether to colorize the debug output
extern _Thread_local _Bool debug_colorize;

/// The amount of indentation to insert before each line of debug output
extern _Thread_local int debug_indent;

/// Whether to debug a type as a negative or positive type
extern _Thread_local _Bool debug_negate;

/// Stream to emit debugging output to (defaults to @c stderr)
extern FILE *debug_stream;

#define PRIsKIND "%s%s%s"
#define DEBUG_KIND(text) \
  debug_colorize ? "\x1b[0;33m" : "", (text), debug_colorize ? "\x1b[0m" : ""
#define DEBUG_CORE_KIND(text) "", (text), ""
#define DEBUG_COERCION_KIND(text) \
  debug_colorize ? "\x1b[0;34m" : "", (text), debug_colorize ? "\x1b[0m" : ""

#define PRIuID "%s%zu%s"
#define DEBUG_ID(id) \
  debug_colorize ? "\x1b[0;31m" : "", (id), debug_colorize ? "\x1b[0m" : ""

#define PRIsNAME "%s"
#define DEBUG_NAME(name) ((name)->text)

#define WITH_DEBUG_INDENT() \
  for (int _i = (debug_indent += 2); debug_indent == _i; debug_indent -= 2)

#define WITH_DEBUG_NEGATE() \
  for (_Bool _n = (debug_negate = !debug_negate); debug_negate == _n; debug_negate = !debug_negate)

#define debug(...) fprintf(debug_stream, ##__VA_ARGS__)

#endif /* MU_STATOR_DEBUG_I */
