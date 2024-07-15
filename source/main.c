#include <muon.h>
#include "analyzer.h"
#include "script.h"
#include "stator.h"
#include "status.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char buffer[4096];

#include "inductor.h"
#include "stator.h"

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
  for (size_t i = 0; i < engine.node_number; i++) {
    if (resolution[i] != NULL)
      fprintf(stderr, "Stator #%zu = Stator #%zu\n", i, resolution[i]->as_stator.id);
  }

  inductor_t *inductor = inductor_initialize(
      &(inductor_t) {0}, &engine, resolution, &status);
  assert(inductor != NULL);

  for (size_t i = 0; i < script->argc; i++) {
    if (induce_node(inductor, &script->argv[i]->as_node) == NULL)
      assert(0);
  }

  size_t length = 1000;
  const mu_type_t **reduce;
  if ((reduce = malloc(sizeof(const mu_type_t *[length]))) == NULL)
    abort();
  for (size_t i = 0; i < length; reduce[i++] = NULL);
  inductor->reduce = reduce;

  for (size_t i = 0; i < script->argc; i++) {
    const mu_node_t *node = &script->argv[i]->as_node;

    do {
      const mu_node_t *next;
      while ((next = node_at(node, node_cursor(node)->i++)) != NULL)
        node = node_continue(node, next);

      const mu_type_t *type;
      if ((type = reduce_node(inductor, node)) == NULL)
        abort();

      fprintf(stderr, "Node %zu: ", node->as_stator.id);
      mu_type_debug(type);
      putc('\n', stderr);
    } while ((node = node_return(node)) != NULL);
  }

  return EXIT_SUCCESS;

except_read_script:
  fclose(stream);

except_fopen:
  return EXIT_FAILURE;
}
