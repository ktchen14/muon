#ifndef MU_READER_SYMBOL_I
#define MU_READER_SYMBOL_I

#include "syntax.h"

#include <muon/status.h>

#include <stdio.h>

typedef struct {
  yytoken_kind_t kind;
  YYSTYPE yylval;
  YYLTYPE yylloc;
} symbol_t;

void symbol_debug(
    FILE *stream,
    yytoken_kind_t kind,
    const YYSTYPE *yylval,
    const YYLTYPE *yylloc)
  __attribute__((nonnull(1)));

#endif /* MU_READER_SYMBOL_I */
