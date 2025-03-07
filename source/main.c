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

  const char name_text[] = "handle_list";
  const mu_name_t *name = mu_name(&engine, strlen(name_text), (const mu_char8_t *) name_text);
  assert(name != NULL);

  const mu_native_expr_t *native_expr = mu_native_expr(&engine, name);
  assert(native_expr != NULL);

  const mu_define_stmt_t *define_stmt = mu_define_stmt(
      &engine, name, &native_expr->as_expr, NULL);
  assert(define_stmt != NULL);

  const mu_stmt_t *prefix[] = { &define_stmt->as_stmt };

  const mu_sequence_expr_t *sequence_expr;
  if ((sequence_expr = mu_script_to_sequence_expr_with_prefix(&engine, script, 1, prefix)) == NULL)
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

  extern _Thread_local _Bool debug_colorize;
  debug_colorize = 1;
  debug_induce = &induce;

  mu_sequence_expr_debug(sequence_expr);

  if (getenv("DOT") != NULL) {
    fprintf(stderr, "digraph muon {\n");
    for (size_t i = 0; i < debug_induce->edge_length; i++) {
      induce_edge_t sub = debug_induce->edge[i];

      fprintf(stderr, "%*s", 2, "");
      fprintf(stderr, "\"");
      debug_just_type(sub.upper);
      fprintf(stderr, "\" -> \"");
      debug_just_type(sub.lower);
      fprintf(stderr, "\";\n");
    }
    fprintf(stderr, "}\n");
  }

  return EXIT_SUCCESS;

except_read_script:
  fclose(stream);

except_fopen:
  return EXIT_FAILURE;
}
