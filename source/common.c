#include "common.h"

#include <stdio.h>

FILE *mu_debug_stream;
_Thread_local _Bool mu_debug_colorize;

_Thread_local int debug_indent;
_Thread_local _Bool debug_negate;
_Thread_local _Bool debug_shortcore;
_Thread_local _Bool debug_dot;

__attribute__((constructor)) static void set_debug_stream(void) {
  mu_debug_stream = stderr;
}
