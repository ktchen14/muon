#ifndef MU_COMMON_H
#define MU_COMMON_H

#include <stdint.h>
#include <stdio.h>

#define MUON_HINT(...) __attribute__((__VA_ARGS__))
#define MUON_HINT_SUFFIX(...) __attribute__((__VA_ARGS__))

/// Stream to emit debugging output to (defaults to @c stderr)
extern _Thread_local FILE *muon_debug_stream;

/// Whether to colorize the debug output
extern _Thread_local _Bool mu_debug_colorize;

typedef uint64_t MuonHash;

#endif /* MU_COMMON_H */
