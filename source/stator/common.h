#ifndef MU_STATOR_COMMON_I
#define MU_STATOR_COMMON_I

#include <muon/stator/common.h>  // IWYU pragma: export

typedef struct mu_engine_t mu_engine_t;
typedef struct mu_name_t mu_name_t;
typedef struct mu_node_t mu_node_t;
typedef struct mu_type_t mu_type_t;

typedef struct {
  mu_engine_t *engine;
  const mu_name_t **name;
  const mu_node_t **node;
  const mu_type_t **type;
} import_t;

#define import_retrieve(x, stator) x[(stator)->as_stator.id]

#endif /* MU_STATOR_COMMON_I */
