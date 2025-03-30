#ifndef MU_COMMON_H
#define MU_COMMON_H

#include <stdio.h>

/// Stream to emit debugging output to (defaults to @c stderr)
extern FILE *mu_debug_stream;

/// Whether to colorize the debug output
extern _Thread_local _Bool mu_debug_colorize;

#endif /* MU_COMMON_H */
