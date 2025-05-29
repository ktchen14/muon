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

unsigned char buffer[4096];

MuonName *vector_access;

int main(int argc, char *argv[argc]) {
#define MU_EMIT(lower, upper, t)
    MU_EACH_EXPR_KIND(MU_EMIT);
#undef MU_EMIT

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

  MuonEngine engine = {0};
  mu_status_t status = {0};

  mu_script_t *script;

  if ((script = mu_read_script(&engine, &status, buffer)) == NULL) {
    fprintf(stderr, "%s: mu_read_script(): %s\n", muon_name, strerror(errno));
    goto except_read_script;
  }

  vector_access = mu_name(&engine, strlen("handle_list"), (mu_char8_t[]) { "handle_list" });
  assert(vector_access != NULL);

  mu_native_expr_t *vector_access_expr = mu_native_expr(&engine, vector_access);
  assert(vector_access_expr != NULL);

  mu_define_stmt_t *define_vector_access = mu_define_stmt(
      &engine, vector_access, &vector_access_expr->as_expr);
  assert(define_vector_access != NULL);

  mu_stmt_t *prefix[] = {
    &define_vector_access->as_stmt,
  };

  mu_sequence_expr_t *sequence_expr;
  if ((sequence_expr = mu_script_to_sequence_expr_with_prefix(&engine, script, 1, prefix)) == NULL)
  // if ((sequence_expr = mu_script_to_sequence_expr(&engine, script)) == NULL)
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

  mu_node_debug(&sequence_expr->as_node);

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

      const mu_coercion_t *coercion;
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

  author_t *author, _author;
  if ((author = author_initialize(&_author, detect_result(&detect), &induce)) == NULL)
    assert(0);
  author->native_expr_emit = standard_native_expr_emit;

  void *module = script_emit(author, &sequence_expr->as_node, argv[1]);
  assert(module != NULL);

  mu_run(module);

  return EXIT_SUCCESS;

except_read_script:
  fclose(stream);

except_fopen:
  return EXIT_FAILURE;
}
