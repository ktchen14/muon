#include <muon.h>
#include "analyzer.h"
#include "script.h"
#include "status.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char buffer[4096];

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

  /* const mu_stmt_t *stmt; */
  /* for (size_t i = 0; i < script->argc; i++) { */
  /*   stmt = script->argv[i]; */
  /*   const mu_sign_t *sign = induce[stmt->as_stator.id]; */
  /*   if (sign != NULL) { */
  /*     fprintf(stderr, "Stator #%zu: ", stmt->as_stator.id); */
  /*     mu_sign_debug(sign); */
  /*   } */
  /* } */

  return EXIT_SUCCESS;

except_read_script:
  fclose(stream);

except_fopen:
  return EXIT_FAILURE;
}
