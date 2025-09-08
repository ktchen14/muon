#include <muon.h>

#include "../source/common.h"
#include "../source/script.h"
#include "../source/status.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  const char *muon = argc > 0 ? argv[0] : "muon";
  
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <input_file>\n", muon);
    return EXIT_FAILURE;
  }

  // Read input file
  char buffer[4096];
  FILE *stream;
  if ((stream = fopen(argv[1], "r")) == NULL) {
    fprintf(stderr, "%s: fopen(): %s\n", muon, strerror(errno));
    return EXIT_FAILURE;
  }

  size_t read = fread(buffer, 1, sizeof(buffer) - 1, stream);
  buffer[read] = '\0';
  fclose(stream);

  // Parse the Muon script
  MuonEngine engine = {0};
  mu_status_t status = {0};

  debug_scan = 1;

  mu_script_t *script;
  if ((script = muon_scan(&engine, &status, buffer)) == NULL) {
    fprintf(stderr, "Parse error\n");
    return EXIT_FAILURE;
  }
  mu_script_debug(script);

  return EXIT_SUCCESS;
}
