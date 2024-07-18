#include "variable_view.h"

#include "engine.h"
#include "name.h"
#include "node.h"

#include <stddef.h>
#include <stdio.h>

const mu_variable_view_t *mu_variable_view(
    mu_engine_t *engine, const mu_node_source_t *source, const mu_name_t *name) {
  size_t size = sizeof(mu_variable_view_t);

  mu_variable_view_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_variable_view_t) {
    .as_view.kind = MU_VARIABLE_VIEW, .name = name,
  };

  if (source != NULL)
    result->as_node.source = *source;

  return assign_node(engine, result);
}

void mu_variable_view_debug(const mu_variable_view_t *view) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, "Variable View #%zu: name = ", view->as_stator.id);
  mu_name_debug(view->name);
  putc('\n', stderr);
}
