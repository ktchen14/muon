#include <muon.h>
#include "analyzer.h"
#include "script.h"
#include "status.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char buffer[4096];

#include "menu.h"
#include "node.h"

criteria_t *induce(const mu_script_t *script, const induce_menu_t *menu) {
  criteria_t *criteria = malloc(sizeof(criteria_t));
  criteria->length = 0;

  for (size_t i = 0; i < script->argc; i++) {
    const mu_node_t *node = &script->argv[i]->as_node;

    do {
      const mu_node_t *next;
      while ((next = node_at(node, node_cursor(node)->i++)) != NULL)
        node = node_continue(node, next);

      criteria = node_induce(node, criteria, menu);
    } while ((node = node_return(node)) != NULL);
  }

  return criteria;
}

int main(int argc, char *argv[argc]) {
  const char *muon_name = argc > 0 ? argv[0] : "muon";
  if (argc < 2) {
    fprintf(stderr, "Usage: %s source\n", muon_name);
    return EXIT_FAILURE;
  }

  FILE *stream;
  if ((stream = fopen(argv[1], "r")) == NULL) {
    fprintf(stderr, "%s: fopen(): %s\n", muon_name, strerror(errno));
    goto except_fopen;
  }

  size_t read;
  read = fread(buffer, 1, 4096, stream);
  buffer[read] = '\0';

  mu_engine_t engine = {0};
  mu_status_t status = {0};

  mu_script_t *script;

  if ((script = mu_read_script(&engine, &status, buffer)) == NULL) {
    fprintf(stderr, "%s: mu_read_script(): %s\n", muon_name, strerror(errno));
    goto except_read_script;
  }

  mu_script_debug(script);

  const mu_stmt_t *const *resolution = resolve_names(
      &engine, script);
  for (size_t i = 0; i < engine.stator_id; i++) {
    if (resolution[i] != NULL)
      fprintf(stderr, "Stator #%zu = Stator #%zu\n", i, resolution[i]->as_stator.id);
  }


  induce_menu_t induce_menu = {
    .engine = &engine,
    .node_to_stmt = resolution,
  };
  criteria_t *criteria = induce(script, &induce_menu);
  assert(criteria != NULL);

  for (size_t i = 0; i < criteria->length; i++) {
    constraint_t *constraint = &criteria->data[i];
    fprintf(stderr, "Constraint: %zu = %zu\n",
        constraint->a.node->as_stator.id,
        constraint->b.node->as_stator.id);
  }


  return EXIT_SUCCESS;

except_read_script:
  fclose(stream);

except_fopen:
  return EXIT_FAILURE;
}
