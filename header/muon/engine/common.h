#ifndef MU_ENGINE_COMMON_H
#define MU_ENGINE_COMMON_H

#include <stddef.h>

typedef const struct mu_name_t mu_name_t;

typedef struct {
  size_t name_number;
  size_t node_number;

  mu_name_t *name[256];
} mu_engine_t;

#endif /* MU_ENGINE_COMMON_H */
