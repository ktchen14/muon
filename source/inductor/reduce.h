#ifndef MUON_INDUCTOR_REDUCE_I
#define MUON_INDUCTOR_REDUCE_I

#include "common.h"

#include "../engine.h"

#include <stddef.h>

typedef struct {
  size_t argc;
  MuonType *argv[];
} Aspect;

MuonType *reduce_node(Inductor *inductor, MuonNode *root)
  MUON_HINT_SUFFIX(nonnull);

[[gnu::nonnull, gnu::pure]] static inline MuonType *type_solution(
    const Inductor *inductor, MuonType *type) {
  if (type->id >= inductor->type_length)
    return NULL;
  return inductor->solution[type->id];
}

#endif /* MUON_INDUCTOR_REDUCE_I */
