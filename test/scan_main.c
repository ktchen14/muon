#include <stdio.h>

#include "reader.h"

const unsigned char source[] =
  "1 2\n32 45"
  "1 2\n32 45"
  "1 2\n32 45"
  "1 2\n32 45"
  "1 2\n32 45"
  "1 2\n32 45"
  "1 2\n32 45"
  "1 2\n32 45"
  "1 2\n32 45"
  "1 2\n32 45"
  "\ntrue";

int main(int argc, char *argv[]) {
  scan_debug(source, NULL);
}
