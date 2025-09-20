#ifndef MU_COMMON_H
#define MU_COMMON_H

#include <stdio.h>

/// Stream to emit debugging output to (defaults to @c stderr)
extern FILE *muon_debug_stream;

/// Whether to colorize the debug output
extern _Thread_local _Bool mu_debug_colorize;

/// @internal Expands to <tt>emit(__VA_ARGS__)</tt>
#define MUON_INDIRECT(emit, ...) emit(__VA_ARGS__)

/// @internal Expands to @a argument
#define MUON_TAKE(argument, ...) argument

#endif /* MU_COMMON_H */
