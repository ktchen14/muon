#include "expr.h"

void mu_expr_debug(const mu_expr_t *expr) {
  switch (expr->kind) {
    case MU_ACCESS_EXPR: 
      return access_expr_debug((const mu_access_expr_t *) expr);

    case MU_INTEGER_EXPR:
      return integer_expr_debug((const mu_integer_expr_t *) expr);

    case MU_MEMBER_EXPR:
      return member_expr_debug((const mu_member_expr_t *) expr);

    case MU_RECORD_EXPR:
      return record_expr_debug((const mu_record_expr_t *) expr);

    case MU_VECTOR_EXPR:
      return vector_expr_debug((const mu_vector_expr_t *) expr);

    case MU_ZERO_EXPR:
      return zero_expr_debug((const mu_zero_expr_t *) expr);
  }
}
