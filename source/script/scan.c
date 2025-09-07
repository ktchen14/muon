#include "syntax.h"

#include "../engine.h"
#include "../script.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*!conditions:re2c*/

typedef mu_char8_t YYCTYPE;

typedef struct {
  size_t cursor;  ///< location of the active character
  size_t marker;
  enum YYCONDTYPE condition;
} scan_t;

#define UTF8(...) ((mu_char8_t *) __VA_ARGS__)

/// Scan and return the next symbol in the @a buffer
yytoken_kind_t scan_next(
    const YYCTYPE *restrict buffer, scan_t *scan, YYSTYPE *yylval, YYLTYPE *yylloc)
  __attribute__((nonnull));

yytoken_kind_t scan_next(
    const YYCTYPE *restrict buffer, scan_t *scan, YYSTYPE *yylval, YYLTYPE *yylloc) {
#define YYPEEK()             buffer[scan->cursor]
#define YYSKIP()             (scan->cursor++)
#define YYBACKUP()           (scan->marker = scan->cursor)
#define YYRESTORE()          (scan->cursor = scan->marker)
#define YYGETCONDITION()     (scan->condition)
#define YYSETCONDITION(next) (scan->condition = next)
#define YYSTAGP(name)        (name = scan->cursor)
#define YYSHIFTSTAG(name, n) (name += n)

  for (;;) {
    /*!stags:re2c format = 'size_t @@;'; */
    /*!svars:re2c format = 'size_t @@ = 0;'; */

    /*!re2c
      re2c:api           = custom;
      re2c:case-ranges   = 1;
      re2c:encoding:utf8 = 1;
      re2c:indent:string = "  ";
      re2c:indent:top    = 1;
      re2c:tags          = 1;
      re2c:yyfill:enable = 0;

      !include "syntax.re";
    */

    __builtin_unreachable();
  }

  return YYEOF;
}
