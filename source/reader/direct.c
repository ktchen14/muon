#include "scan.h"
#include "syntax.h"

#include "../expr.h"
#include "../name.h"
#include "../stmt.h"

#include <assert.h>

const mu_name_t *handle_name(mu_engine_t *engine) {
  const mu_name_t *name;
  name = mu_name(engine, 1, (const mu_char8_t *) "a");
  return name;
}

const mu_expr_t *handle_expr(mu_engine_t *engine) {
  const mu_integer_expr_t *integer_expr;
  integer_expr = mu_integer_expr(engine, 1);
  return &integer_expr->as_expr;
}

#define YYCTYPE char8_t

/// Scan and return the next symbol in the @a buffer
yytoken_kind_t next(scan_t *scan, YYSTYPE *yylval)
  __attribute__((nonnull));

yytoken_kind_t peek(scan_t *scan)
  __attribute__((nonnull));

const mu_constant_stmt_t *handle_constant_stmt(
    mu_engine_t *engine, scan_t *scan) {
  YYSTYPE yylval;
  yytoken_kind_t kind = next(scan, &yylval);
  assert(kind == CONSTANT);

  if ((kind = peek(scan)) == ' ')
    next(scan, &yylval);
  else {
    /* error */
  }

  const mu_name_t *name;
  if ((kind = peek(scan)) == NAME) {
    next(scan, &yylval);
    if ((name = mu_name(engine, yylval.text.length, yylval.text.c)) == NULL)
      return NULL;
  } else {
  }

  if ((kind = next(scan, &yylval)) != '=') {
    abort();
  }
}

/* const mu_stmt_t *handle_stmt( */
/*     mu_engine_t *engine, */
/*     scan_t *scan) { */
/*   yytoken_kind_t kind; */
/*   YYSTYPE yylval; */
/*   YYLTYPE yylloc; */

/*   kind = peek(scan); */

/*   if (kind == CONSTANT) { */
/*   } */
/* } */
