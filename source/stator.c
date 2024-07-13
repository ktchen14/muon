#include "stator.h"

void mu_node_debug(const mu_node_t *node) {
  switch (node->kind) {
#define MU_EMIT(lower, upper, t) \
    case MU_##upper##_NODE: \
      mu_##lower##_debug((const mu_##lower##_t *) node); \
      break;
    MU_EACH_NODE_KIND(MU_EMIT)
#undef MU_EMIT
  }
}

void mu_expr_debug(const mu_expr_t *expr) {
  switch (expr->kind) {
#define MU_EMIT(lower, upper, t) \
    case MU_##upper##_EXPR: \
      mu_##lower##_expr_debug((const mu_##lower##_expr_t *) expr); \
      break;
    MU_EACH_EXPR_KIND(MU_EMIT)
#undef MU_EMIT
  }
}

void mu_sign_debug(const mu_sign_t *sign) {
  switch (sign->kind) {
#define MU_EMIT(lower, upper, t) \
    case MU_##upper##_SIGN: \
      mu_##lower##_sign_debug((const mu_##lower##_sign_t *) sign); \
      break;
    MU_EACH_SIGN_KIND(MU_EMIT)
#undef MU_EMIT
  }
}

void mu_stmt_debug(const mu_stmt_t *stmt) {
  switch (stmt->kind) {
#define MU_EMIT(lower, upper, t) \
    case MU_##upper##_STMT: \
      mu_##lower##_stmt_debug((const mu_##lower##_stmt_t *) stmt); \
      break;
    MU_EACH_STMT_KIND(MU_EMIT)
#undef MU_EMIT
  }
}

void mu_type_debug(const mu_type_t *type) {
  switch (type->kind) {
#define MU_EMIT(lower, upper, t) \
    case MU_##upper##_TYPE: \
      mu_##lower##_type_debug((const mu_##lower##_type_t *) type); \
      break;
  MU_EACH_TYPE_KIND(MU_EMIT)
#undef MU_EMIT
  }
}
