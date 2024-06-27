#include "expr.h"

void mu_expr_debug(const mu_expr_t *expr) {
  switch (expr->kind) {
    case MU_ACCESS_EXPR: 
      mu_access_expr_debug((const mu_access_expr_t *) expr);
      break;

    case MU_INTEGER_EXPR:
      mu_integer_expr_debug((const mu_integer_expr_t *) expr);
      break;

    case MU_MEMBER_EXPR:
      mu_member_expr_debug((const mu_member_expr_t *) expr);
      break;

    case MU_NAME_EXPR:
      mu_name_expr_debug((const mu_name_expr_t *) expr);
      break;

    case MU_RECORD_EXPR:
      mu_record_expr_debug((const mu_record_expr_t *) expr);
      break;

    case MU_VECTOR_EXPR:
      mu_vector_expr_debug((const mu_vector_expr_t *) expr);
      break;

    case MU_ZERO_EXPR:
      mu_zero_expr_debug((const mu_zero_expr_t *) expr);
      break;
  }
}
