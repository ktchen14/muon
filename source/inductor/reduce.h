#ifndef MU_INDUCTOR_REDUCE_I
#define MU_INDUCTOR_REDUCE_I

#include <muon/stator.h>

#include "induce.h"

#include <assert.h>
#include <stddef.h>

typedef struct {
  inductor_t *inductor; 
  const mu_type_t **data;
} reduce_t;

const mu_type_t *reduce_type(reduce_t *reduce, const mu_type_t *type)
  __attribute__((nonnull));

const mu_type_t *reduce_node(reduce_t *reduce, const mu_node_t *node)
  __attribute__((nonnull));

#endif /* MU_INDUCTOR_REDUCE_I */
