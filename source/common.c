#include "common.h"

#include <stdio.h>

_Thread_local _Bool debug_colorize;
_Thread_local int debug_indent;
_Thread_local _Bool debug_negate;
_Thread_local _Bool debug_shortcore;
FILE *debug_stream;

__attribute__((constructor)) static void set_debug_stream(void) {
  debug_stream = stderr;
}
