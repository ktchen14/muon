#include "syntax.h"

#include "../common.h"

#include <stdio.h>

const char *symbol_name(yytoken_kind_t kind);

void symbol_debug(
    FILE *stream, yytoken_kind_t kind, YYSTYPE *yylval, YYLTYPE *yylloc) {
  debug("%s:%zu:%zu [%zu + %zu]: ",
      yylloc->name != NULL ? yylloc->name : "(none)",
      yylloc->line,
      yylloc->column,
      yylloc->offset,
      yylloc->length);
  debug("%s", symbol_name(kind));
  debug("\n");
}
