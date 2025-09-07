#include "syntax.h"
#include "symbol.h"
#include "scan.h"

#include "../script.h"
#include "../engine.h"
#include "../status.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void scan_debug(const YYCTYPE *string, const char *name) {
  scan_t scan = {0};
  yytoken_kind_t kind;
  YYSTYPE yylval;
  YYLTYPE yylloc;

  while ((kind = scan_next(string, &scan, &yylval, &yylloc)) != YYEOF)
    symbol_debug(stderr, kind, &yylval, &yylloc);
}

mu_script_t *mu_read_script(
    MuonEngine *engine, mu_status_t *status, const mu_char8_t *string) {
  syntax_t syntax = { .engine = engine };
  scan_t scan = {0};

  // Initialize the Bison parser
  yypstate *pstate;
  if ((pstate = yypstate_new()) == NULL)
    return errno = ENOMEM, NULL;

  int e;
  do {
    YYSTYPE yylval;
    YYLTYPE yylloc;
    yytoken_kind_t kind = scan_next(string, &scan, &yylval, &yylloc);
    /* symbol_debug(stderr, kind, &yylval, &yylloc); */
    e = yypush_parse(pstate, kind, &yylval, &yylloc, &syntax);
  } while (e == YYPUSH_MORE);

  yypstate_delete(pstate);

  if ((errno = ((int[]) {0, EINVAL, ENOMEM})[e]) != 0)
    fprintf(stderr, "Error %i\n", e);

  return syntax.script;
}