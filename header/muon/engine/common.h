#ifndef MUON_ENGINE_COMMON_H
#define MUON_ENGINE_COMMON_H

#include "../common.h" // IWYU pragma: export

#include <limits.h>
#include <stddef.h>

typedef struct {
  void *remote;
  _Alignas(max_align_t) char data[sizeof(void *[256])];
} MuonEngine;

#endif /* MUON_ENGINE_COMMON_H */
