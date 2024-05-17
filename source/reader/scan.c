#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../common.h"

#include "syntax.h"
#include "symbol.h"

#include <muon/status.h>

/*!conditions:re2c*/

#define YYCTYPE char8_t

typedef struct {
  size_t offset, line, column;
} cursor_t;

typedef struct {
  cursor_t cursor;  ///< location of the active character
  cursor_t symbol;  ///< location of the active symbol
  cursor_t marker;
  enum YYCONDTYPE condition;
} scan_t;

/// Scan and return the next symbol in the @a buffer
static yytoken_kind_t scan_next(
    const YYCTYPE *restrict buffer, scan_t *scan, YYSTYPE *yylval)
  __attribute__((nonnull));

/// Advance the scan @a cursor
static void cursor_next(
    const YYCTYPE *restrict buffer, cursor_t *cursor)
  __attribute__((nonnull));

void scan_debug(const YYCTYPE *string, const char *name) {
  scan_t scan = {0};

  for (yytoken_kind_t kind;;) {
    YYSTYPE yylval;
    if ((kind = scan_next(string, &scan, &yylval)) == YYEOF)
      break;

    symbol_t symbol = {
      .kind = kind,
      .yylval = yylval,
      .yylloc = {
        .name = name,
        .offset = scan.symbol.offset,
        .length = scan.cursor.offset - scan.symbol.offset,
        .line = scan.symbol.line,
        .column = scan.symbol.column,
      },
    };

    symbol_debug(stderr, &symbol);
  }
}

static yytoken_kind_t scan_next(
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

#define UTF8(...) ((char8_t *) __VA_ARGS__)

    /*!re2c
      re2c:api = custom;
      re2c:indent:string = "  ";
      re2c:yyfill:enable = 0;

      !include "syntax.re";
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

static void cursor_next(
    const YYCTYPE *restrict buffer, cursor_t *cursor) {
  if (buffer[cursor->offset++] == '\n') {
    cursor->line++;
    cursor->column = 1;
  } else
    cursor->column++;
}
