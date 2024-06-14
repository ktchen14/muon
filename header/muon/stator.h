#ifndef MU_STATOR_H
#define MU_STATOR_H

#include "common.h"

#include <stddef.h>

typedef struct mu_engine_t mu_engine_t;

/**
 * @brief An enumeration of each kind of stator
 */
typedef enum {
  MU_NAME_STATOR,

  // Type
  MU_RECORD_TYPE_STATOR,
  MU_VECTOR_TYPE_STATOR,

  // Expr
  MU_ACCESS_EXPR_STATOR,
  MU_INTEGER_EXPR_STATOR,
  MU_MEMBER_EXPR_STATOR,
  MU_RECORD_EXPR_STATOR,
  MU_VECTOR_EXPR_STATOR,
  MU_ZERO_EXPR_STATOR,

  // Stmt
  MU_CONSTANT_STMT_STATOR,
} mu_stator_kind_t;

/// An abstract stator
typedef struct {
  mu_stator_kind_t kind;

  const mu_engine_t *engine;
  size_t id;
} mu_stator_t;

/* /// An enumeration of each kind of type */
/* typedef enum { */
/*   MU_RECORD_TYPE = MU_RECORD_TYPE_NODE, */
/*   MU_VECTOR_TYPE = MU_VECTOR_TYPE_NODE, */
/* } mu_type_kind_t; */

#endif /* MU_STATOR_H */
