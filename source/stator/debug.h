#ifndef MU_STATOR_DEBUG_I
#define MU_STATOR_DEBUG_I

#include "node.h"

#include "../inductor/induce.h"

#include <stdio.h>

/// Whether to colorize the debug output
extern _Thread_local _Bool debug_colorize;

/// The amount of indentation to insert before each line of debug output
extern _Thread_local int debug_indent;

/// Whether to debug a type as a negative or positive type
extern _Thread_local _Bool debug_negate;

#define PRIsKIND "%s%s%s"
#define DEBUG_KIND(text) \
  debug_colorize ? "\e[0;33m" : "", (text), debug_colorize ? "\e[0m" : ""
#define DEBUG_COERCION_KIND(text) \
  debug_colorize ? "\e[0;34m" : "", (text), debug_colorize ? "\e[0m" : ""

#define PRIuID "%s%zu%s"
#define DEBUG_ID(id) \
  debug_colorize ? "\e[0;31m" : "", (id), debug_colorize ? "\e[0m" : ""

#define WITH_DEBUG_INDENT() \
  for (int _i = (debug_indent += 2); debug_indent == _i; debug_indent -= 2)

#define WITH_DEBUG_NEGATE() \
  for (_Bool _n = (debug_negate = !debug_negate); debug_negate == _n; debug_negate = !debug_negate)

__attribute__((nonnull))
static inline void debug_node_type(const mu_node_t *node) {
  if (debug_induce == NULL)
    return;

  const mu_type_t *type;
  if ((type = induce_reveal(debug_induce, node)) == NULL)
    return;

  fprintf(stderr, " ∷ ");

  mu_variable_type_t sentinel;
  type_link_t link = { .next = &sentinel };
  mark_type_from_anywhere_first(debug_induce, type, &link);

  if (link.next != &sentinel) {
    size_t number = 0;
    for (const mu_variable_type_t *type = link.next; type != &sentinel; type = type->debug_next) {
      if (is_significant(type))
        number++;
    }

    if (number > 0) {
      fprintf(stderr, "∃(");

      size_t i = 0;
      for (const mu_variable_type_t *type = link.next; type != &sentinel; type = type->debug_next) {
        if (is_significant(type)) {
          if (i++ > 0)
            fprintf(stderr, ", ");
          debug_variable_type_name(type);
        }
      }

      fprintf(stderr, ") ");
    }
  }

  debug_type(type);

  const mu_variable_type_t *next = link.next;
  while (next != &sentinel) {
    ((mu_variable_type_t *) next)->negatively_entered_from = NULL;
    ((mu_variable_type_t *) next)->positively_entered_from = NULL;

    const mu_variable_type_t *real_next = next->debug_next;
    ((mu_variable_type_t *) next)->debug_next = NULL;
    next = real_next;
  }
}

__attribute__((nonnull))
static inline void expr_debug_with_coercion(const mu_expr_t *expr) {
  int i = debug_indent;

  if (debug_induce != NULL) {
    const mu_coercion_t *coercion;
    if ((coercion = debug_induce->coercion[expr->as_node.id]) != NULL) {
      fprintf(stderr, "%*s", debug_indent, "");
      mu_coercion_debug(coercion);
      fprintf(stderr, ":\n");
      debug_indent += 2;
    }
  }

  mu_expr_debug(expr);

  debug_indent = i;
}

#endif /* MU_STATOR_DEBUG_I */
