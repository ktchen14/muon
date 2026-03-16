#ifndef MUON_COMMON_H
#define MUON_COMMON_H

#include <stdint.h>
#include <stdio.h>

/// @internal Expands to <tt>__attribute__((...))</tt>
#define MUON_HINT(...) __attribute__((__VA_ARGS__))

/// @internal Expands to <tt>__attribute__((...))</tt>
#define MUON_HINT_SUFFIX(...) __attribute__((__VA_ARGS__))

/// @internal Expands to <tt>emit(...)</tt>
#define MUON_INDIRECT(emit, ...) emit(__VA_ARGS__)

/// @internal Expands to @a argument
#define MUON_TAKE(argument, ...) argument

/// Stream to emit debugging output to (defaults to @c stderr)
extern _Thread_local FILE *muon_debug_stream;

/// Whether to colorize the debug output
extern _Thread_local _Bool muon_debug_colorize;

#endif /* MUON_COMMON_H */
