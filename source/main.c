#include <muon.h>

#include "common.h"
#include "author.h"
#include "engine.h"
#include "inductor.h"
#include "runner.h"
#include "script.h"
#include "standard.h"
#include "status.h"

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[/* argc */]) {
  debug_scan = 1;

  const char *main_name = argc > 0 ? argv[0] : "muon";
  if (argc < 2) {
    fprintf(stderr, "Usage: %s source\n", main_name);
    return EXIT_FAILURE;
  }

  char buffer[4096];
  FILE *stream;
  if ((stream = fopen(argv[1], "r")) == NULL) {
    fprintf(stderr, "%s: fopen(): %s\n", main_name, strerror(errno));
    goto except_fopen;
  }

  size_t read;
  read = fread(buffer, 1, 4096, stream);
  buffer[read] = '\0';

  MuonEngine engine = {0};
  mu_status_t status = {0};

  mu_script_t *script;

  if ((script = muon_scan(&engine, &status, buffer)) == NULL) {
    fprintf(stderr, "%s: muon_scan(): %s\n", main_name, strerror(errno));
    goto except_scan;
  }

  MuonSequenceExpr *sequence_expr;
  if ((sequence_expr = mu_script_to_sequence_expr(&engine, script)) == NULL)
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

  extern _Thread_local _Bool mu_debug_colorize;
  mu_debug_colorize = 1;
  debug_induce = &induce;

  muon_node_debug(&sequence_expr->as_node);

  if (getenv("DOT") != NULL) {
    FILE *output = fopen("out.dot", "w");
    mu_debug_stream = output;
    mu_debug_colorize = 0;
    debug_shortcore = 1;
    debug_dot = 1;

    debug("digraph muon {\n  rankdir=\"BT\";\n  dpi=192;\n");
    for (size_t i = 0; i < debug_induce->universe.length; i++) {
      type_edge_t edge = debug_induce->universe.data[i];

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

      if (edge.indirect || edge.transitive)
        debug(" [constraint=false]");
      if (edge.indirect)
        debug(" [style=dashed]");

      MuonCoercion *coercion;
      if ((coercion = course_coercion(&edge)) != NULL) {
        debug(" [label=\" ");
        mu_coercion_debug(edge.coercion);
        debug("\"]");
      }

      debug(";\n");
    }
    debug("}\n");

    fclose(output);
    mu_debug_stream = stderr;

    system("dot -Tpng -O out.dot");
  }

  if (getenv("LLVM") != NULL) {
    author_t *author, _author;
    if ((author = author_initialize(&_author, detect_result(&detect), &induce)) == NULL)
      assert(0);
    author->native_expr_emit = standard_native_expr_emit;

    void *module = script_emit(author, &sequence_expr->as_node, argv[1]);
    assert(module != NULL);

    mu_run(module);
  }

  return EXIT_SUCCESS;

except_scan:
  fclose(stream);

except_fopen:
  return EXIT_FAILURE;
}
