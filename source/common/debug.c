#include "debug.h"

#include <stdio.h>

_Thread_local FILE *muon_debug_stream;
_Thread_local _Bool muon_debug_colorize;

_Thread_local _Bool debug_scan;
