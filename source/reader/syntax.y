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

script: constant_stmt {
}

constant_stmt: "constant" _ name _ '=' _ expr {
}

// ================================== Expr =====================================

expr: integer_expr

integer_expr: INTEGER {
  $$ = mu_integer_expr(engine, $1, &@$);
}

name: NAME {
  $$ = mu_name(engine, $1.length, $1.c);
}

_: ' '

%%
