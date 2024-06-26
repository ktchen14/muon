#include <muon.h>
#include "reader.h"
#include "script.h"
#include "status.h"

#include <stdlib.h>

const char muon[] = "constant x Integer = 1\n";

int main(int argc, char *argv[]) {
  mu_engine_t engine = {0};
  mu_status_t status = {0};

  mu_script_t *script = mu_read_script(&engine, &status, muon);
  mu_script_debug(script);

  return EXIT_SUCCESS;
}
