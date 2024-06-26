#include "expr.h"

void mu_expr_debug(const mu_expr_t *expr) {
  switch (expr->kind) {
    case MU_ACCESS_EXPR: 
      return mu_access_expr_debug((const mu_access_expr_t *) expr);

    case MU_INTEGER_EXPR:
      return mu_integer_expr_debug((const mu_integer_expr_t *) expr);

    case MU_MEMBER_EXPR:
      return mu_member_expr_debug((const mu_member_expr_t *) expr);

    case MU_NAME_EXPR:
      return mu_name_expr_debug((const mu_name_expr_t *) expr);

    case MU_RECORD_EXPR:
      return mu_record_expr_debug((const mu_record_expr_t *) expr);

    case MU_VECTOR_EXPR:
      return mu_vector_expr_debug((const mu_vector_expr_t *) expr);

    case MU_ZERO_EXPR:
      return mu_zero_expr_debug((const mu_zero_expr_t *) expr);
  }
}
