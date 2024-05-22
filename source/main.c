#include "muon/engine.h"
#include "name.h"

#include <stdlib.h>

int main(int argc, char *argv[]) {
  mu_engine_t engine = {0};

  const mu_name_t *name;
  if ((name = mu_name(&engine, (char8_t *) "asdf", 4)) == NULL)
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
