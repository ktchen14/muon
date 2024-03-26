#include <stdio.h>

#include "reader.h"

const unsigned char source[] = "1 2\n32 45";

int main(int argc, char *argv[]) {
  scan_string_debug(source);
}
