#include "common.h"

#include <stdio.h>

_Thread_local FILE *muon_debug_stream;
_Thread_local _Bool mu_debug_colorize;

_Thread_local int debug_indent;
_Thread_local _Bool debug_negate;
_Thread_local _Bool debug_shortcore;
_Thread_local _Bool debug_dot;
_Thread_local _Bool debug_scan;
