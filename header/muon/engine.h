#ifndef MU_ENGINE_H
#define MU_ENGINE_H

#include "common.h"

#include <stddef.h>

typedef struct mu_engine_t mu_engine_t;
typedef struct mu_name_t mu_name_t;

struct mu_engine_t {
  size_t stator_id;

  const mu_name_t *name[256];
  size_t name_i;
};

#endif /* MU_ENGINE_H */
