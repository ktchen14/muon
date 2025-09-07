#ifndef MUON_SCRIPT_SYMBOL_H_INCLUDED
#define MUON_SCRIPT_SYMBOL_H_INCLUDED

#include "syntax.h"

#include <stdio.h>

void symbol_debug(
    FILE *stream, yytoken_kind_t kind, YYSTYPE *yylval, YYLTYPE *yylloc)
  __attribute__((nonnull));

#endif