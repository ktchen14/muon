#ifndef MU_ENGINE_COMMON_H
#define MU_ENGINE_COMMON_H

#include "name.h"

#include <stddef.h>

typedef struct mu_engine_t mu_engine_t;
struct mu_engine_t {
  size_t name_number;
  size_t node_number;

  const mu_name_t *name[256];
};

#endif /* MU_ENGINE_COMMON_H */
