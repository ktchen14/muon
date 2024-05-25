#ifndef MU_STATOR_H
#define MU_STATOR_H

#include "common.h"
#include "status.h"

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
  MU_OBJECT_EXPR_STATOR,
  MU_VECTOR_EXPR_STATOR,

  // Stmt
  MU_CONSTANT_STMT_STATOR,
} mu_stator_kind_t;

/// An abstract stator
typedef struct {
  mu_stator_kind_t kind;
  const mu_engine_t *engine;
  size_t id;
} mu_stator_t;

/// An enumeration of each kind of node
typedef enum {
  // Type
  MU_RECORD_TYPE_NODE = MU_RECORD_TYPE_STATOR,
  MU_VECTOR_TYPE_NODE = MU_VECTOR_TYPE_STATOR,

  // Expr
  MU_OBJECT_EXPR_NODE = MU_OBJECT_EXPR_STATOR,
  MU_VECTOR_EXPR_NODE = MU_VECTOR_EXPR_STATOR,

  // Stmt
  MU_CONSTANT_STMT_NODE = MU_CONSTANT_STMT_STATOR,
} mu_node_kind_t;

/// An enumeration of each kind of type
typedef enum {
  MU_RECORD_TYPE = MU_RECORD_TYPE_NODE,
  MU_VECTOR_TYPE = MU_VECTOR_TYPE_NODE,
} mu_type_kind_t;

/// An enumeration of each kind of expr
typedef enum {
  MU_OBJECT_EXPR = MU_OBJECT_EXPR_NODE,
  MU_VECTOR_EXPR = MU_VECTOR_EXPR_NODE,
} mu_expr_kind_t;

/// An enumeration of each kind of stmt
typedef enum {
  MU_CONSTANT_STMT = MU_CONSTANT_STMT_NODE,
} mu_stmt_kind_t;

typedef struct mu_node_t mu_node_t;

typedef struct {
  const mu_node_t *node;
  size_t i;
} node_cursor_t;

struct mu_node_t {
  union {
    mu_node_kind_t kind;
    mu_stator_t as_stator;
  };
  node_cursor_t cursor;
  mu_source_t source;
};

typedef struct {
  union {
    mu_expr_kind_t kind;
    mu_node_t as_node;
  };
} mu_expr_t;

typedef struct {
  mu_expr_t as_expr;
} mu_constant_expr_t;

#define engine(stator) _Generic((stator), \
  const mu_stator_t *: (stator)->engine, \
  mu_stator_t *: (stator)->engine, \
  const mu_node_t *: (stator)->as_stator.engine, \
  mu_node_t *: (stator)->as_stator.engine, \
  const mu_expr_t *: (stator)->as_node.as_stator.engine, \
  mu_expr_t *: (stator)->as_node.as_stator.engine, \
  const mu_constant_expr_t *: (stator)->as_expr.as_node.as_stator.engine, \
  mu_constant_expr_t *: (stator)->as_expr.as_node.as_stator.engine, \
)

#endif /* MU_STATOR_H */
