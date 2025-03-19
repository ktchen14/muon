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

#include "debug.h"
#include <inttypes.h>

void mu_node_debug1(const mu_node_t *node) {
  // Kind -> Text, e.g. [MU_ACCESS_EXPR_NODE] = "AccessExpr"
  static const char *const TEXT[] = {
#define MU_EMIT(l, upper, title) [MU_##upper##_NODE] = #title,
    MU_EACH_NODE_KIND(MU_EMIT)
#undef MU_EMIT
  };

  fprintf(stderr, "%*s" PRIsKIND "#" PRIuID,
      debug_indent, "", DEBUG_KIND(TEXT[node->kind]), DEBUG_ID(node->id));

  switch ON_ABSTRACT_OBJECT(node) {
    case IS_KIND_OF(access_expr):
      debug(" (name = " PRIsNAME ")", DEBUG_NAME(access_expr->name)); break;

    case IS_KIND_OF(boolean_expr):
      debug(" (data = %s)", boolean_expr->data ? "true" : "false"); break;

    case IS_KIND_OF(integer_expr):
      debug(" (data = %" PRIu64 ")", integer_expr->data); break;

    case IS_KIND_OF(name_expr):
      debug(" (name = " PRIsNAME ")", DEBUG_NAME(name_expr->name)); break;

    case IS_KIND_OF(native_expr):
      debug(" (name = " PRIsNAME ")", DEBUG_NAME(native_expr->name)); break;

    case IS_KIND_OF(expr_member):
      if (expr_member->name != NULL)
        debug(" (name = " PRIsNAME ")", DEBUG_NAME(expr_member->name));
      break;

    case IS_KIND_OF(switch_case):
      debug(" (name = " PRIsNAME ")", DEBUG_NAME(switch_case->name)); break;

    case IS_KIND_OF(name_sign):
      debug(" (name = " PRIsNAME ")", DEBUG_NAME(name_sign->name)); break;

    case IS_KIND_OF(datatype_option):
      debug(" (name = " PRIsNAME ")", DEBUG_NAME(datatype_option->name)); break;

    case IS_KIND_OF(datatype_stmt):
      debug(" (name = " PRIsNAME ")", DEBUG_NAME(datatype_stmt->name)); break;

    case IS_KIND_OF(define_stmt):
      debug(" (name = " PRIsNAME ")", DEBUG_NAME(define_stmt->name)); break;

    case IS_KIND_OF(variable_view):
      debug(" (name = " PRIsNAME ")", DEBUG_NAME(variable_view->name)); break;

    default: break;
  }

  debug_node_type(node);
  putc('\n', stderr);

  WITH_DEBUG_INDENT() {
    size_t i = 0;
    for (const mu_node_t *next; (next = node_at(node, i)) != NULL; i++)
      mu_node_debug1(next);
  }
}
