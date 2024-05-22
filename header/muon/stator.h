#ifndef MU_STATOR_H
#define MU_STATOR_H

#include "common.h"

#include <stddef.h>

typedef struct mu_engine_t mu_engine_t;

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

typedef enum {
  MU_RECORD_TYPE = MU_RECORD_TYPE_NODE,
  MU_VECTOR_TYPE = MU_VECTOR_TYPE_NODE,
} mu_type_kind_t;

typedef enum {
  MU_OBJECT_EXPR = MU_OBJECT_EXPR_NODE,
  MU_VECTOR_EXPR = MU_VECTOR_EXPR_NODE,
} mu_expr_kind_t;

typedef enum {
  MU_CONSTANT_STMT = MU_CONSTANT_STMT_NODE,
} mu_stmt_kind_t;

/// An abstract stator
typedef struct {
  const mu_engine_t *engine;
  mu_stator_kind_t kind;
} mu_stator_t;

#define MU_STATOR_HEADER(kind_type) \
  union { \
    struct { \
      const mu_engine_t *engine; \
      kind_type kind; \
    }; \
    mu_stator_t as_stator; \
  }

typedef struct mu_node_t mu_node_t;

typedef struct {
  const mu_node_t *node;
  size_t i;
} node_cursor_t;

struct mu_node_t {
  MU_STATOR_HEADER(mu_node_kind_t);
  node_cursor_t cursor;
};

#define MU_NODE_HEADER \
  union { \
    struct { \
      MU_STATOR_HEADER(mu_node_kind_t); \
      node_cursor_t cursor; \
    }; \
    mu_node_t as_node; \
  }

typedef struct {
  MU_NODE_HEADER;
} mu_constant_node_t;

#endif /* MU_STATOR_H */
