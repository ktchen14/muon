#include <muon.h>

#include "inductor.h"
#include "script.h"
#include "stator.h"
#include "status.h"

#include "common.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char buffer[4096];

#include "stator.h"

const mu_name_t *vector_access;
const mu_name_t *vector_join;

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

  vector_access = mu_name(&engine, strlen("handle_list"), (mu_char8_t[]) { "handle_list" });
  assert(vector_access != NULL);

  const mu_native_expr_t *vector_access_expr = mu_native_expr(&engine, vector_access);
  assert(vector_access_expr != NULL);

  const mu_define_stmt_t *define_vector_access = mu_define_stmt(
      &engine, vector_access, &vector_access_expr->as_expr);
  assert(define_vector_access != NULL);

  vector_join = mu_name(&engine, strlen("+"), (mu_char8_t[]) { "+" });
  assert(vector_join != NULL);

  const mu_native_expr_t *vector_join_expr = mu_native_expr(&engine, vector_join);
  assert(vector_join_expr != NULL);

  const mu_define_stmt_t *define_vector_join = mu_define_stmt(
      &engine, vector_join, &vector_join_expr->as_expr);
  assert(define_vector_join != NULL);

  const mu_stmt_t *prefix[] = {
    &define_vector_access->as_stmt,
    &define_vector_join->as_stmt,
  };

  const mu_sequence_expr_t *sequence_expr;
  if ((sequence_expr = mu_script_to_sequence_expr_with_prefix(&engine, script, 2, prefix)) == NULL)
    assert(0);

  detect_t detect;
  if (detect_initialize(&detect, &engine, &status) == NULL)
    assert(0);

  if (detect_node(&detect, &sequence_expr->as_node) == NULL)
    assert(0);

  induce_t induce;
  if (induce_initialize(&induce, &engine, &detect) == NULL)
    assert(0);

  if (induce_node(&induce, &sequence_expr->as_node) == NULL)
    assert(0);

  if (reduce_node(&induce, &sequence_expr->as_node) == NULL)
    assert(0);

  extern _Thread_local _Bool debug_colorize;
  debug_colorize = 1;
  debug_induce = &induce;

  mu_node_debug(&sequence_expr->as_node);

  if (getenv("DOT") != NULL) {
    FILE *output = fopen("out.dot", "w");
    debug_stream = output;
    debug_colorize = 0;
    debug_shortcore = 1;

    debug("digraph muon {\n  rankdir=\"BT\";\n  dpi=192;\n");
    for (size_t i = 0; i < debug_induce->universe.length; i++) {
      induce_edge_t edge = debug_induce->universe.data[i];

      /* if (edge.indirect == 1) */
      /*   continue; */

      debug("%*s", 2, "");

      debug("\"");
      type_debug(edge.source, 0);
      debug(" #%zu", edge.source->id);
      debug("\"");

      debug(" -> ");

      debug("\"");
      type_debug(edge.target, 0);
      debug(" #%zu", edge.target->id);
      debug("\"");

      if (edge.indirect == 2)
        debug(" [constraint=false,style=dashed,color=darkgreen]");
      else if (edge.indirect == 1)
        debug(" [constraint=false,style=dashed]");

      if (edge.coercion != NULL) {
        debug(" [label=\" ");
        const mu_edge_coercion_t *edge_coercion;
        if ((edge_coercion = mu_coercion_cast(edge.coercion, edge_coercion)) != NULL) {
          assert(edge_coercion->source == edge.source);
          assert(edge_coercion->as_coercion.target == edge.target);
          debug("*");
        } else
          mu_coercion_debug(edge.coercion);
        debug("\"]");
      }

      debug(";\n");
    }
    debug("}\n");

    fclose(output);
    debug_stream = stderr;

    system("dot -Tpng -O out.dot");
  }

  return EXIT_SUCCESS;

except_read_script:
  fclose(stream);

except_fopen:
  return EXIT_FAILURE;
}
