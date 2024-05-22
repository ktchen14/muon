#ifndef MU_NODE_COMMON_H
#define MU_NODE_COMMON_H

#include "../stator.h"
#include "../status.h"

/**
 * @brief An abstract node
 */
typedef struct {
  union {
    mu_stator_t as_abstract_stator;

    struct {
      const mu_engine_t *engine;
      mu_node_kind_t kind;
    };
  };

  mu_source_t source;
} mu_node_t;

#endif /* MU_NODE_COMMON_H */
