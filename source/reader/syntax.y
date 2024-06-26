%require "3.8.0"

// ================================ Prologue ============================== {{{1

%code requires {
#include <muon/status.h>
#include "../common.h"
#include "../expr.h"
#include "../name.h"
#include "../script.h"
#include "../sign.h"
#include "../stmt.h"

typedef struct {
  mu_engine_t *engine;
  mu_script_t *script;
} syntax_t;
}

%define api.location.type { mu_source_t }
%define api.pure full
%define api.push-pull push
%locations
%parse-param { syntax_t *syntax }
%start script

// ============================== Declaration ============================= {{{1

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
  const mu_vector_expr_t *vector_expr;

  const mu_sign_t *sign;
  const mu_integer_sign_t *integer_sign;
  const mu_vector_sign_t *vector_sign;

  const mu_stmt_t *stmt;
  const mu_constant_stmt_t *constant_stmt;
}

%token CONSTANT "constant"
%token INSTANCE "instance"
%token TYPE "type"
%token INTEGER "Integer"

%token <integer> INTEGER_LITERAL
%token <boolean> BOOLEAN_LITERAL
%token <text>    STRING
%token <text>    NAME

%type <name> name

%type <expr> expr
%type <access_expr> access_expr
%type <integer_expr> integer_expr
%type <vector_expr> vector_expr

%type <sign> sign
%type <integer_sign> integer_sign
%type <vector_sign> vector_sign

%type <stmt> stmt
%type <constant_stmt> constant_stmt

// ========================= YYLLOC_DEFAULT/yyerror ======================= {{{1

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

static void yyerror(YYLTYPE *yylloc, syntax_t *syntax, char const *s);
%}

// ================================= Script =============================== {{{1

%%

script: stmt {
  syntax->script = mu_script(1, &$stmt);
}

// ================================== Expr ================================ {{{1

expr: '(' expr ')' { $$ = $2; } |
  access_expr  { $$ = &$access_expr->as_expr; } |
  integer_expr { $$ = &$integer_expr->as_expr; } |
  vector_expr  { $$ = &$vector_expr->as_expr; }

access_expr: expr '.' name {
  $$ = mu_access_expr(syntax->engine, $name, $expr, &@$);
}

integer_expr: INTEGER_LITERAL {
  $$ = mu_integer_expr(syntax->engine, $1, &@$);
}

vector_expr: '[' expr[argv] ']' {
  $$ = mu_vector_expr(syntax->engine, 1, &$argv);
}

// ================================== Name ================================ {{{1

name: NAME {
  $$ = mu_name(syntax->engine, $1.length, $1.c);
}

// ================================== Sign ================================ {{{1

sign: '(' sign ')' { $$ = $2; } |
  integer_sign { $$ = &$integer_sign->as_sign; } |
  vector_sign  { $$ = &$vector_sign->as_sign; }

integer_sign: "Integer" {
  $$ = mu_integer_sign(syntax->engine, &@$);
}

vector_sign: '[' sign ']' {
  $$ = mu_vector_sign(syntax->engine, $sign, &@$);
}

// ================================== Stmt ================================ {{{1

stmt:
  constant_stmt { $$ = &$constant_stmt->as_stmt; }

constant_stmt: "constant" _ name _ sign _ '=' _ expr '\n' {
  $$ = mu_constant_stmt(syntax->engine, $name, $expr, $sign);
}

// ============================= Miscellaneous ============================ {{{1

_: ' '

%%

static void yyerror(YYLTYPE *yylloc, syntax_t *syntax, char const *s) {
  fprintf(stderr, "%s\n", s);
}

// vim: set foldlevel=1 foldmethod=marker:
