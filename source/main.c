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
  if (induce_initialize(&induce, &engine, &status, &detect) == NULL)
    assert(0);

  if (induce_node(&induce, &sequence_expr->as_node) == NULL)
    assert(0);

  if (reduce_node(&induce, &sequence_expr->as_node) == NULL)
    assert(0);

  extern _Thread_local _Bool debug_colorize;
  debug_colorize = 1;
  debug_induce = &induce;

  mu_sequence_expr_debug(sequence_expr);

  if (getenv("DOT") != NULL) {
    fprintf(stderr, "digraph muon {\n  rankdir=\"BT\"\n");
    for (size_t i = 0; i < debug_induce->edge_length; i++) {
      induce_edge_t sub = debug_induce->edge[i];

      fprintf(stderr, "%*s", 2, "");

      fprintf(stderr, "\"");
      debug_just_type(sub.lower);
      fprintf(stderr, " (%zu)", sub.lower->id);
      fprintf(stderr, "\"");

      fprintf(stderr, " -> ");

      fprintf(stderr, "\"");
      debug_just_type(sub.upper);
      fprintf(stderr, " (%zu)", sub.upper->id);
      fprintf(stderr, "\"");

      if (!sub.direct)
        fprintf(stderr, " [constraint=false,style=dashed]");

      /* if (sub.coercion != NULL && strlen(sub.coercion) != 0) */
      /*   fprintf(stderr, " [label=\"%s\"]", sub.coercion); */
      fprintf(stderr, ";\n");
    }
    fprintf(stderr, "}\n");
  }

  return EXIT_SUCCESS;

except_read_script:
  fclose(stream);

except_fopen:
  return EXIT_FAILURE;
}
