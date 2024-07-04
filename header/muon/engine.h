#ifndef MU_ENGINE_H
#define MU_ENGINE_H

#include "common.h"

#include <stddef.h>

typedef struct mu_engine_t mu_engine_t;
typedef struct mu_name_t mu_name_t;

struct mu_engine_t {
  size_t name_number;
  size_t node_number;
  size_t type_number;

  const mu_name_t *name[256];
};

#endif /* MU_ENGINE_H */
