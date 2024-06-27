#ifndef MU_NODE_I
#define MU_NODE_I

#include "node/common.h"

#include "expr.h"
#include "stmt.h"

#include <stddef.h>
#include <stdlib.h>

static inline const mu_node_t *node_at(const mu_node_t *node, size_t i) {
  switch (node->kind) {
    case MU_ACCESS_EXPR_NODE: 
      return access_expr_at((const mu_access_expr_t *) node, i);

    case MU_INTEGER_EXPR_NODE:
      return integer_expr_at((const mu_integer_expr_t *) node, i);

    case MU_MEMBER_EXPR_NODE:
      return member_expr_at((const mu_member_expr_t *) node, i);

    case MU_NAME_EXPR_NODE:
      return name_expr_at((const mu_name_expr_t *) node, i);

    case MU_RECORD_EXPR_NODE:
      return record_expr_at((const mu_record_expr_t *) node, i);

    case MU_VECTOR_EXPR_NODE:
      return vector_expr_at((const mu_vector_expr_t *) node, i);

    case MU_ZERO_EXPR_NODE:
      return zero_expr_at((const mu_zero_expr_t *) node, i);

    case MU_INTEGER_SIGN_NODE:
    case MU_MEMBER_SIGN_NODE:
    case MU_NAME_SIGN_NODE:
    case MU_RECORD_SIGN_NODE:
    case MU_VARIABLE_SIGN_NODE:
    case MU_VECTOR_SIGN_NODE:
      abort();

    case MU_CONSTANT_STMT_NODE:
      return constant_stmt_at((const mu_constant_stmt_t *) node, i);
  }

  return NULL;  // TODO: unreachable
}

#endif /* MU_NODE_I */
