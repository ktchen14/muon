#include <stdio.h>

#include "reader.h"

const unsigned char source[] =
  "true false\n"
  "0 1234\n"
  "\"abcd\\\"\\t\\n\\r\""
  "kaiting\n";

int main(int argc, char *argv[]) {
  scan_debug(source, NULL);
}
