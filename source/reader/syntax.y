%require "3.8.0"

%code requires {
#include <muon/common.h>
#include <muon/status.h>
#include <muon/name.h>
#include <muon/expr.h>
}

%define api.location.type { mu_source_t }
%define api.pure full
%define api.push-pull push
%locations
%parse-param { mu_engine_t *engine }

%union {
  long long integer;
  _Bool boolean;

  struct {
    const mu_char8_t *c;
    size_t length;
  } text;

  const mu_name_t *name;
  const mu_expr_t *expr;

  const mu_access_expr_t *access_expr;
  const mu_integer_expr_t *integer_expr;
}

%token CONSTANT "constant"
%token INSTANCE "instance"
%token TYPE "type"

%token <integer> INTEGER
%token <boolean> BOOLEAN
%token <text>    STRING
%token <text>    NAME

%type <name> name
%type <expr> expr

%type <access_expr> access_expr
%type <integer_expr> integer_expr

%start script

%{
#include <assert.h>
#include <stdio.h>

#define YYLLOC_DEFAULT(result, argv, n) do { \
  if (n) { \
    assert(YYRHSLOC((argv), 1).name == YYRHSLOC((argv), n).name); \
    (result).name   = YYRHSLOC((argv), n).name; \
    \
    (result).offset = YYRHSLOC((argv), 1).offset; \
    (result).line   = YYRHSLOC((argv), 1).line; \
    (result).column = YYRHSLOC((argv), 1).column; \
    \
    (result).length = YYRHSLOC((argv), n).offset \
                    + YYRHSLOC((argv), n).length \
                    - YYRHSLOC((argv), 1).offset; \
  } else \
    (result) = YYRHSLOC((argv), 0); \
} while (0)

void yyerror(YYLTYPE *yylloc, mu_engine_t *engine, char const *s) {
  fprintf(stderr, "%s\n", s);
}
%}

%%

script: {
} | script stmt {
}

stmt: constant_stmt

constant_stmt: "constant" _ name _ '=' _ expr '\n' {
}

// ================================== Expr =====================================

expr: '(' expr ')' { $$ = $2; } |
  access_expr  { $$ = &$access_expr->as_expr; } |
  integer_expr { $$ = &$integer_expr->as_expr; }

access_expr: expr '.' name {
  $$ = mu_access_expr(engine, $name, $expr, &@$);
}

integer_expr: INTEGER {
  $$ = mu_integer_expr(engine, $1, &@$);
}

// ================================== Name =====================================

name: NAME {
  $$ = mu_name(engine, $1.length, $1.c);
}

_: ' '

%%
