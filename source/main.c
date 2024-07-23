#include <muon.h>

#include "inductor.h"
#include "script.h"
#include "stator.h"
#include "status.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char buffer[4096];

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

  const mu_sequence_expr_t *sequence_expr;
  if ((sequence_expr = mu_script_to_sequence_expr(&engine, script)) == NULL)
    assert(0);

  mu_sequence_expr_debug(sequence_expr);

  detect_t detect;
  if (detect_initialize(&detect, &engine, &status) == NULL)
    assert(0);

  if (detect_node(&detect, &sequence_expr->as_node) == NULL)
    assert(0);

  induce_t induce;
  if (induce_initialize(&induce, &engine, &status, &detect) == NULL)
    assert(0);

  if (induce_node(&induce, &sequence_expr->as_node) == NULL)
    assert(0);

  /* reduce_t reduce = { .induce = &induce }; */
  /* size_t length = 1000; */
  /* const mu_type_t **reduce_data; */
  /* if ((reduce_data = malloc(sizeof(const mu_type_t *[length]))) == NULL) */
  /*   abort(); */
  /* for (size_t i = 0; i < length; reduce_data[i++] = NULL); */
  /* reduce.data = reduce_data; */

  /* const mu_node_t *node = &sequence_expr->as_node; */

  /* do { */
  /*   const mu_node_t *next; */
  /*   while ((next = node_at(node, node_cursor(node)->i++)) != NULL) { */
  /*     if (reduce.data[node->as_stator.id] != NULL) */
  /*       continue; */
  /*     node = node_continue(node, next); */
  /*   } */

  /*   const mu_type_t *type; */
  /*   if ((type = reduce_node(&reduce, node)) == NULL) */
  /*     abort(); */

  /*   fprintf(stderr, "Node %zu: ", node->as_stator.id); */
  /*   mu_type_debug(type); */
  /*   putc('\n', stderr); */
  /* } while ((node = node_return(node)) != NULL); */

  return EXIT_SUCCESS;

except_read_script:
  fclose(stream);

except_fopen:
  return EXIT_FAILURE;
}
