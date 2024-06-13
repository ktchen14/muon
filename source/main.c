#include "muon/engine.h"
#include "name.h"

#include <stdlib.h>

int main(int argc, char *argv[]) {
  mu_engine_t engine = {0};

  const mu_name_t *name;
  if ((name = mu_name(&engine, 4, (char8_t *) "asdf")) == NULL)
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
