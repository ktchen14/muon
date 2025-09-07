#include <muon.h>

#include "../source/common.h"
#include "../source/script.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  const char *program_name = argc > 0 ? argv[0] : "test_runner";
  
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <input_file>\n", program_name);
    return EXIT_FAILURE;
  }

  // Read input file
  unsigned char buffer[4096];
  FILE *stream;
  if ((stream = fopen(argv[1], "r")) == NULL) {
    fprintf(stderr, "%s: fopen(): %s\n", program_name, strerror(errno));
    return EXIT_FAILURE;
  }

  size_t read = fread(buffer, 1, sizeof(buffer) - 1, stream);
  buffer[read] = '\0';
  fclose(stream);

  // Parse the Muon script
  MuonEngine engine = {0};

  mu_script_t *script = muon_scan(&engine, NULL, (const char*)buffer);
  if (script == NULL) {
    fprintf(stderr, "Parse error\n");
    return EXIT_FAILURE;
  }

  // For now, just print success and basic info about the parsed script
  printf("Parse successful\n");
  printf("Statements: %zu\n", script->argc);

  // Debug output if requested
  if (getenv("DEBUG")) {
    mu_script_debug(script);
  }

  return EXIT_SUCCESS;
}