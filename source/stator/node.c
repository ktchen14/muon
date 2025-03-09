#include "node.h"

void mu_node_debug(const mu_node_t *node) {
  switch (node->kind) {
#define MU_EMIT(lower, upper, t) \
    case MU_##upper##_NODE: \
      mu_##lower##_debug((const mu_##lower##_t *) node); return;
    MU_EACH_NODE_KIND(MU_EMIT)
#undef MU_EMIT
  }
  __builtin_unreachable();
}

void mu_expr_debug(const mu_expr_t *expr) {
  switch (expr->kind) {
#define MU_EMIT(lower, upper, t) \
    case MU_##upper##_EXPR: \
      mu_##lower##_expr_debug((const mu_##lower##_expr_t *) expr); return;
    MU_EACH_EXPR_KIND(MU_EMIT)
#undef MU_EMIT
  }
  __builtin_unreachable();
}

void mu_sign_debug(const mu_sign_t *sign) {
  switch (sign->kind) {
#define MU_EMIT(lower, upper, t) \
    case MU_##upper##_SIGN: \
      mu_##lower##_sign_debug((const mu_##lower##_sign_t *) sign); return;
    MU_EACH_SIGN_KIND(MU_EMIT)
#undef MU_EMIT
  }
  __builtin_unreachable();
}

void mu_stmt_debug(const mu_stmt_t *stmt) {
  switch (stmt->kind) {
#define MU_EMIT(lower, upper, t) \
    case MU_##upper##_STMT: \
      mu_##lower##_stmt_debug((const mu_##lower##_stmt_t *) stmt); return;
    MU_EACH_STMT_KIND(MU_EMIT)
#undef MU_EMIT
  }
  __builtin_unreachable();
}

void mu_view_debug(const mu_view_t *view) {
  switch (view->kind) {
#define MU_EMIT(lower, upper, t) \
    case MU_##upper##_VIEW: \
      mu_##lower##_view_debug((const mu_##lower##_view_t *) view); return;
    MU_EACH_VIEW_KIND(MU_EMIT)
#undef MU_EMIT
  }
  __builtin_unreachable();
}
