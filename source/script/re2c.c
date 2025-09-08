#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// Return the next symbol in the @a scan
__attribute__((nonnull))
static yytoken_kind_t symbol(Scan *scan, YYSTYPE *yylval, YYLTYPE *yylloc) {
  typedef unsigned char YYCTYPE;

  for (;;) {
    /*!stags:re2c format = 'size_t @@;'; */
    /*!svars:re2c format = 'size_t @@ = 0;'; */
    /*!conditions:re2c*/

    /*!re2c
      re2c:api                = custom;
      re2c:api:style          = free-form;
      re2c:case-ranges        = 1;
      re2c:encoding:utf8      = 1;
      re2c:indent:string      = "  ";
      re2c:indent:top         = 2;
      re2c:tags               = 1;
      re2c:yyfill:enable      = 0;

      re2c:YYPEEK             = "scan->text[scan->cursor]";
      re2c:YYSKIP             = "scan->cursor++;";
      re2c:YYBACKUP           = "scan->marker = scan->cursor;";
      re2c:YYRESTORE          = "scan->cursor = scan->marker;";
      re2c:YYGETCONDITION     = "scan->mode";
      re2c:YYSETCONDITION     = "scan->mode = @@;";
      re2c:YYSTAGP            = "@@{tag} = scan->cursor;";
      re2c:YYSHIFTSTAG        = "@@{tag} += @@{shift};";

      !include "syntax.re2c";
    */

    __builtin_unreachable();
  }

  return YYEOF;
}
