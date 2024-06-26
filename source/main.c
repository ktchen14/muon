#include "muon/engine.h"
#include "name.h"
#include "expr.h"

#include <stdlib.h>

const char muon[] = "constant x = 1\n";

int main(int argc, char *argv[]) {
  mu_engine_t engine = {0};

  const mu_name_t *name;
  if ((name = mu_name(&engine, 4, (char8_t *) "asdf")) == NULL)
    return EXIT_FAILURE;

  const mu_integer_expr_t *integer_expr = mu_integer_expr(&engine, 1, NULL);
  if (integer_expr == NULL)
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
