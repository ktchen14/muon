#include <muon.h>

#include "author.h"
#include "common.h"
#include "detector.h"
#include "engine.h"
#include "inductor.h"
#include "runner.h"
#include "scan.h"
#include "standard.h"
#include "status.h"

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static MuonType *node_type(MuonNode *node, const void *data) {
  const MuonInductor *inductor = data;

  MuonType *type;
  if ((type = node_source_type(inductor, node)) == NULL)
    return NULL;

  // return type;

  MuonType *solution;
  if ((solution = type_solution(inductor, type)) == NULL)
    return type;
  return solution;
}

int main(int argc, char *argv[/* argc */]) {
  // debug_scan = 1;

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

  MuonEngine *engine;
  if ((engine = muon_engine_initialize(&(MuonEngine) {})) == NULL)
    assert(0);
  mu_status_t status = {0};

  MuonModule *module = muon_standard_module(engine);
  if (module == NULL)
    assert(0);

  MuonScript *script;

  if ((script = muon_scan(engine, &status, buffer)) == NULL) {
    fprintf(stderr, "%s: muon_scan(): %s\n", main_name, strerror(errno));
    goto except_scan;
  }

  detect_t detect;
  if (detect_initialize(&detect, engine, &status, module) == NULL)
    assert(0);

  if (detect_node(&detect, &script->as_node) == NULL)
    assert(0);

  MuonInductor inductor;
  if (muon_induce_initialize(&inductor, engine, &detect, module) == NULL)
    assert(0);

  if (induce_script(&inductor, script) == NULL)
    assert(0);

  if (reduce_node(&inductor, &script->as_node) == NULL)
    assert(0);

  muon_debug_colorize = 1;

  muon_node_debug(&script->as_node, .type = node_type, .type_data = &inductor);

  if (getenv("DOT") != NULL) {
    FILE *output = fopen("out.dot", "w");
    muon_debug_stream = output;
    muon_debug_colorize = 0;

    inductor_debug(&inductor);

    fclose(output);
    muon_debug_stream = stderr;

    system("dot -Tpng -O out.dot"); // NOLINT(bugprone-command-processor)
  }

  // if (getenv("LLVM") != NULL) {
  //   author_t *author, _author;
  //   if ((author = author_initialize(&_author, detect_result(&detect),
  //   &induce))
  //       == NULL)
  //     assert(0);
  //   author->native_expr_emit = standard_native_expr_emit;
  //
  //   void *module = script_emit(author, &script->as_node, argv[1]);
  //   assert(module != NULL);
  //
  //   mu_run(module);
  // }

  return EXIT_SUCCESS;

except_scan:
  fclose(stream);

except_fopen:
  return EXIT_FAILURE;
}
