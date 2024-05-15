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

void symbol_debug(FILE *stream, const symbol_t *symbol)
  __attribute__((nonnull));

#endif /* MU_READER_SYMBOL_I */
