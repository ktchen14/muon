#include "common.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  assert(argc == 2);

  FILE *stream;
  if ((stream = fopen(argv[1], "w")) == NULL) {
    perror("fopen()");
    return EXIT_FAILURE;
  }

  static const char template[] = "#ifndef MUON_ENGINE_OBJECT_H\n"
                                 "#define MUON_ENGINE_OBJECT_H\n"
                                 "\n"
                                 "struct MuonEngine {\n"
                                 "  _Alignas(%zu) char data[%zu];\n"
                                 "};\n"
                                 "\n"
                                 "#endif /* MUON_ENGINE_OBJECT_H */\n";
  fprintf(stream, template, _Alignof(Engine), sizeof(Engine));
  return EXIT_SUCCESS;
}
