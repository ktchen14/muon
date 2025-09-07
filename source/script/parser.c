#include "syntax.h"
#include "symbol.h"
#include "scan.h"

#include "../common.h"
#include "../engine.h"
#include "../script.h"
#include "../status.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

mu_script_t *mu_read_script(
    MuonEngine *engine, mu_status_t *status, const mu_char8_t *string) {
  syntax_t syntax = { .engine = engine };
  scan_t scan = {0};

  // Initialize the Bison parser
  yypstate *pstate;
  if ((pstate = yypstate_new()) == NULL)
    return errno = ENOMEM, NULL;

  int e;
  yytoken_kind_t kind;
  do {
    YYSTYPE yylval;
    YYLTYPE yylloc;
    kind = scan_next(string, &scan, &yylval, &yylloc);

    if (debug_scan)
      symbol_debug(stderr, kind, &yylval, &yylloc);

    e = yypush_parse(pstate, kind, &yylval, &yylloc, &syntax);
  } while (e == YYPUSH_MORE);

  yypstate_delete(pstate);

  if (kind != YYEOF && debug_scan) {
    yytoken_kind_t kind;
    YYSTYPE yylval;
    YYLTYPE yylloc;
    while ((kind = scan_next(string, &scan, &yylval, &yylloc)) != YYEOF)
      symbol_debug(stderr, kind, &yylval, &yylloc);
  }

  if ((errno = ((int[]) {0, EINVAL, ENOMEM})[e]) != 0)
    fprintf(stderr, "Error %i\n", e);

  return syntax.script;
}
