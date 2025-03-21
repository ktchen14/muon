#ifndef MU_STATOR_DEBUG_I
#define MU_STATOR_DEBUG_I

#include "node.h"

#include "../inductor/induce.h"

#include <stdarg.h>
#include <stdio.h>

/// Whether to colorize the debug output
extern _Thread_local _Bool debug_colorize;

/// The amount of indentation to insert before each line of debug output
extern _Thread_local int debug_indent;

/// Whether to debug a type as a negative or positive type
extern _Thread_local _Bool debug_negate;

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

static inline void debug(const char *restrict format, ...) {
  va_list variadic;
  va_start(variadic, format);
  vfprintf(stderr, format, variadic);
  va_end(variadic);
}

__attribute__((nonnull))
static inline void debug_node_type(const mu_node_t *node) {
  if (debug_induce == NULL)
    return;

  const mu_type_t *type;
  if ((type = induce_reveal(debug_induce, node)) == NULL)
    return;

  fprintf(stderr, " ∷ ");

  type_debug(type, 1);
}

#endif /* MU_STATOR_DEBUG_I */
