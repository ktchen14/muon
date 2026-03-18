#ifndef MUON_ENGINE_COMMON_H
#define MUON_ENGINE_COMMON_H

#include "../common.h" // IWYU pragma: export

#if __has_include(<muon/engine/object.h>)
#include <muon/engine/object.h>
#endif

/// @internal Expands to <tt>emit(...)</tt>
#define MUON_INDIRECT(emit, ...) emit(__VA_ARGS__)

/// @internal Expands to @a argument
#define MUON_TAKE(argument, ...) argument

typedef struct MuonEngine MuonEngine;

#endif /* MUON_ENGINE_COMMON_H */
