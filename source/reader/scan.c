#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "syntax.h"
#include "symbol.h"

#include <muon/status.h>

/*!conditions:re2c*/

#define YYCTYPE unsigned char

typedef struct {
  size_t offset, line, column;
} cursor_t;

typedef struct {
  cursor_t cursor;
  cursor_t symbol;
  cursor_t marker;
  enum YYCONDTYPE condition;
} scan_t;

static yytoken_kind_t scan_next_internal(
    const YYCTYPE *restrict buffer, scan_t *scan, YYSTYPE *yylval)
  __attribute__((nonnull));

static yytoken_kind_t scan_next(
    const YYCTYPE *restrict buffer, scan_t *scan, YYSTYPE *yylval, YYLTYPE *yylloc)
  __attribute__((nonnull));

static void cursor_next(
    const YYCTYPE *restrict buffer, cursor_t *cursor)
  __attribute__((nonnull));

void scan_debug(const YYCTYPE *string) {
  scan_t scan = {0};
  YYSTYPE yylval;
  YYLTYPE yylloc;

  for (yytoken_kind_t kind;;) {
    if ((kind = scan_next(string, &scan, &yylval, &yylloc)) == YYEOF)
      break;
    symbol_debug(stdout, kind, &yylval, &yylloc);
  }
}

static yytoken_kind_t scan_next_internal(
    const YYCTYPE *restrict buffer, scan_t *scan, YYSTYPE *yylval) {
  cursor_t *cursor = &scan->cursor;
  cursor_t *symbol = &scan->symbol;
  cursor_t *marker = &scan->marker;
  enum YYCONDTYPE *condition = &scan->condition;

  for (;;) {
    *symbol = *cursor;

#define YYPEEK()             buffer[cursor->offset]
#define YYSKIP()             cursor_next(buffer, cursor)
#define YYBACKUP()           (*marker = *cursor)
#define YYRESTORE()          (*cursor = *marker)
#define YYGETCONDITION()     (*condition)
#define YYSETCONDITION(next) (*condition = next)

    /*!re2c
      re2c:indent:string = "  ";
      re2c:yyfill:enable = 0;

      !include "reader.re";
    */

#undef YYPEEK
#undef YYSKIP
#undef YYBACKUP
#undef YYRESTORE
#undef YYGETCONDITION
#undef YYSETCONDITION

    assert(0);
  }

  return YYEOF;
}

static yytoken_kind_t scan_next(
    const YYCTYPE *restrict buffer, scan_t *scan, YYSTYPE *yylval, YYLTYPE *yylloc) {
  yytoken_kind_t result = scan_next_internal(buffer, scan, yylval);

  yylloc->offset = scan->symbol.offset;
  yylloc->length = scan->cursor.offset - scan->symbol.offset;
  yylloc->line = scan->symbol.line;
  yylloc->column = scan->symbol.column;

  return result;
}

static void cursor_next(
    const YYCTYPE *restrict buffer, cursor_t *cursor) {
  /* assert(buffer[cursor->offset] != '\0'); */

  if (buffer[cursor->offset] == '\n') {
    cursor->line++;
    cursor->column = 1;
  } else
    cursor->column++;

  cursor->offset++;
}
