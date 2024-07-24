#ifndef MU_STATOR_ENGINE_H
#define MU_STATOR_ENGINE_H

#include "name.h"

#include <stddef.h>

typedef struct mu_engine_t mu_engine_t;
struct mu_engine_t {
  size_t name_number;
  size_t node_number;
  size_t type_number;

  const mu_name_t *name[256];
};

#endif /* MU_STATOR_ENGINE_H */
