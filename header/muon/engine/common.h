#ifndef MUON_ENGINE_COMMON_H
#define MUON_ENGINE_COMMON_H

#include <stddef.h>

typedef const struct MuonName MuonName;

typedef struct {
  size_t name_number;
  size_t node_number;

  MuonName *name[256];
} MuonEngine;

#endif /* MUON_ENGINE_COMMON_H */
