#ifndef MUON_ENGINE_COMMON_H
#define MUON_ENGINE_COMMON_H

#include "../common.h" // IWYU pragma: export

#include <limits.h>
#include <stddef.h>

typedef struct {
  void *remote;
  _Alignas(max_align_t) char data[sizeof(void *[256])];
} MuonEngine;

/// @internal Used to emit each branch in MUON_STATOR_ENUMERATOR(), etc.
#define MUON_ENUMERATOR_EMIT(Title, l, UPPER, SUFFIX) \
  , Muon##Title *: MUON_##UPPER##SUFFIX \
  , struct Muon##Title *: MUON_##UPPER##SUFFIX

#endif /* MUON_ENGINE_COMMON_H */
