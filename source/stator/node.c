#include "debug.h"
#include "node.h"

#include <inttypes.h>

__attribute__((nonnull))
static inline void node_debug_with_coercion(const mu_node_t *node) {
  int i = debug_indent;

  if (debug_induce != NULL) {
    const mu_coercion_t *coercion;
    if ((coercion = debug_induce->coercion[node->id]) != NULL) {
      if (coercion->kind != MU_ID_COERCION) {
        fprintf(stderr, "%*s", debug_indent, "");
        mu_coercion_debug(coercion);

        if (coercion->target != NULL) {
          fprintf(stderr, " ∷ ");
          debug_type(coercion->target);
        }

        fprintf(stderr, "\n");
        debug_indent += 2;
      }
    }
  }

  mu_node_debug(node);

  debug_indent = i;
}

/// Emit debugging information on the abstract @a node to the debug stream
void mu_node_debug(const mu_node_t *node) {
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
      node_debug_with_coercion(next);
  }
}
