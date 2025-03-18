#include "syntax.h"

#include "../common.h"
#include "../script.h"
#include "../status.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*!conditions:re2c*/

#define YYCTYPE mu_char8_t

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
static void cursor_next(const YYCTYPE *restrict buffer, cursor_t *cursor)
  __attribute__((nonnull));

static void symbol_debug(
    FILE *stream, yytoken_kind_t kind, YYSTYPE *yylval, YYLTYPE *yylloc)
  __attribute__((nonnull));

void scan_debug(const YYCTYPE *string, const char *name) {
  scan_t scan = {0};

  yytoken_kind_t kind;
  YYSTYPE yylval;
  YYLTYPE yylloc;

  while ((kind = scan_next(string, &scan, &yylval)) != YYEOF) {
    yylloc = (YYLTYPE) {
      .name = name,
      .offset = scan.symbol.offset,
      .length = scan.cursor.offset - scan.symbol.offset,
      .line = scan.symbol.line,
      .column = scan.symbol.column,
    };

    symbol_debug(stderr, kind, &yylval, &yylloc);
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

#define UTF8(...) ((mu_char8_t *) __VA_ARGS__)

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

static void cursor_next(const YYCTYPE *restrict buffer, cursor_t *cursor) {
  if (buffer[cursor->offset++] == '\n') {
    cursor->line++;
    cursor->column = 1;
  } else
    cursor->column++;
}

static void symbol_debug(
    FILE *stream, yytoken_kind_t kind, YYSTYPE *yylval, YYLTYPE *yylloc) {
  printf("%s:%zu:%zu [%zu + %zu]: ",
      yylloc->name != NULL ? yylloc->name : "(none)",
      yylloc->line,
      yylloc->column,
      yylloc->offset,
      yylloc->length);

  if (kind > YYEOF && kind < YYerror) {
    int c = kind;
    switch (c) {
      case '\n':
        printf("\\n");
        break;
      case ' ':
        printf("_");
        break;
      default:
        printf("%c", c);
        break;
    }
  } else switch (kind) {
    case YYEOF:
      printf("EOF\n");
      break;
    case YYerror:
      printf("Error\n");
      break;
    case DATATYPE:
      printf("\"datatype\"");
      break;
    case DEFINE:
      printf("\"define\"");
      break;
    case INSTANCE:
      printf("\"instance\"");
      break;
    case NAME:
      fputs("NAME ", stdout);
      fwrite(yylval->text.c, yylval->text.length, 1, stdout);
      break;
    case BOOLEAN_LITERAL:
      printf("BOOLEAN LITERAL %s", yylval->boolean ? "true" : "false");
      break;
    case INTEGER_LITERAL:
      printf("INTEGER LITERAL %lli", yylval->integer);
      break;
    case STRING:
      printf("STRING \"");
      for (size_t i = 0; i < yylval->text.length; i++) {
        char c = yylval->text.c[i];
        switch (c) {
          case '"': printf("\\\""); break;
          case '\t': printf("\\t"); break;
          case '\n': printf("\\n"); break;
          case '\r': printf("\\r"); break;
          default: printf("%c", c); break;
        }
      }
      printf("\"");
      break;
    default:
      printf("Unknown %d", kind);
      break;
  }

  printf("\n");
}





mu_script_t *mu_read_script(
    mu_engine_t *engine, mu_status_t *status, const mu_char8_t *string) {
  syntax_t syntax = { .engine = engine };
  scan_t scan = {0};

  // Initialize the Bison parser
  yypstate *pstate;
  if ((pstate = yypstate_new()) == NULL) {
    errno = ENOMEM;
    goto except_yypstate_new;
  }

  int e;
  do {
    YYSTYPE yylval;
    yytoken_kind_t kind = scan_next(string, &scan, &yylval);

    YYLTYPE yylloc = (YYLTYPE) {
      /* .name = name, */
      .offset = scan.symbol.offset,
      .length = scan.cursor.offset - scan.symbol.offset,
      .line = scan.symbol.line,
      .column = scan.symbol.column,
    };

    /* symbol_debug(stderr, kind, &yylval, &yylloc); */

    e = yypush_parse(pstate, kind, &yylval, &yylloc, &syntax);
  } while (e == YYPUSH_MORE);

  switch (e) {
    case 0:  break;
    case 2:  errno = ENOMEM; goto except_yypush_parse;
    default: errno = EINVAL; goto except_yypush_parse;
  }

  yypstate_delete(pstate);
  return syntax.script;

except_yypush_parse:
  fprintf(stderr, "Error %i\n", e);
  // TODO
  /* assert(syntax.script == NULL); */

  yypstate_delete(pstate);

except_yypstate_new:
  return NULL;
}

