%require "3.8.0"

// ================================ Prologue ============================== {{{1

%code requires {
#include <muon/stator.h>
#include "../script.h"

typedef struct {
  mu_engine_t *engine;
  mu_script_t *script;
  const mu_stmt_t *stmt[256];
  size_t stmt_i;
  const mu_expr_t *expr[800];
  size_t expr_i;
  const mu_expr_member_t *expr_member[800];
  size_t expr_member_i;
  const mu_datatype_option_t *datatype_option[200];
  size_t datatype_option_i;
} syntax_t;
}

%define api.location.type { mu_source_t }
%define api.pure full
%define api.push-pull push
%define parse.error detailed
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

  size_t i;

  const mu_name_t *name;

  const mu_expr_t *expr;
  const mu_access_expr_t *access_expr;
  const mu_boolean_expr_t *boolean_expr;
  const mu_integer_expr_t *integer_expr;
  const mu_invoke_expr_t *invoke_expr;
  const mu_lambda_expr_t *lambda_expr;
  const mu_name_expr_t *name_expr;
  const mu_expr_member_t *expr_member;
  const mu_record_expr_t *record_expr;
  const mu_vector_expr_t *vector_expr;

  const mu_sign_t *sign;
  const mu_boolean_sign_t *boolean_sign;
  const mu_integer_sign_t *integer_sign;
  const mu_name_sign_t *name_sign;
  const mu_vector_sign_t *vector_sign;

  const mu_stmt_t *stmt;
  const mu_datatype_option_t *datatype_option;
  const mu_datatype_stmt_t *datatype_stmt;
  const mu_define_stmt_t *define_stmt;

  const mu_view_t *view;
  const mu_variable_view_t *variable_view;
}

%token DATATYPE "datatype"
%token DEFINE "define"
%token INSTANCE "instance"
%token LAMBDA "lambda"

%token BOOLEAN "Boolean"
%token INTEGER "Integer"

%token <integer> INTEGER_LITERAL
%token <boolean> BOOLEAN_LITERAL
%token <text>    STRING
%token <text>    NAME

%type <name> name
%type <expr> expr
%type <sign> sign
%type <stmt> stmt
%type <view> view

%type <access_expr> access_expr
%type <boolean_expr> boolean_expr
%type <integer_expr> integer_expr
%type <invoke_expr> invoke_expr
%type <lambda_expr> lambda_expr
%type <name_expr> name_expr
%type <expr_member> expr_member
%type <record_expr> record_expr
%type <vector_expr> vector_expr

%type <boolean_sign> boolean_sign
%type <integer_sign> integer_sign
%type <name_sign> name_sign
%type <vector_sign> vector_sign

%type <datatype_option> datatype_option
%type <datatype_stmt> datatype_stmt
%type <define_stmt> define_stmt

%type <variable_view> variable_view

%type <i> datatype_argv record_argv vector_argv

%nonassoc LAMBDA
%left     INVOKE ' '
%left     '.'

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

script: script_argv {
  syntax->script = mu_script(syntax->stmt_i, syntax->stmt);
}

script_argv: {
  syntax->stmt_i = 0;

} | script_argv stmt {
  syntax->stmt[syntax->stmt_i++] = $stmt;
}

// ================================== Name ================================ {{{1

name: NAME {
  $$ = mu_name(syntax->engine, $1.length, $1.c);
}

// ================================== Expr ================================ {{{1

expr: '(' expr[matter] ')' { $$ = $matter; } |
  access_expr  { $$ = &$access_expr->as_expr; } |
  boolean_expr { $$ = &$boolean_expr->as_expr; } |
  integer_expr { $$ = &$integer_expr->as_expr; } |
  invoke_expr  { $$ = &$invoke_expr->as_expr; } |
  lambda_expr  { $$ = &$lambda_expr->as_expr; } |
  name_expr    { $$ = &$name_expr->as_expr; } |
  record_expr  { $$ = &$record_expr->as_expr; } |
  vector_expr  { $$ = &$vector_expr->as_expr; }

access_expr: expr[matter] '.' name {
  $$ = mu_access_expr(syntax->engine, $name, $matter);
}

boolean_expr: BOOLEAN_LITERAL {
  $$ = mu_boolean_expr(syntax->engine, $1);
}

integer_expr: INTEGER_LITERAL {
  $$ = mu_integer_expr(syntax->engine, $1);
}

invoke_expr: expr[operator] _ expr[argument] %prec INVOKE {
  $$ = mu_invoke_expr(syntax->engine, $operator, $argument);
}

lambda_expr: "lambda" _ view[argument] _ '=' _ expr[matter] %prec LAMBDA {
  $$ = mu_lambda_expr(syntax->engine, $argument, $matter);
}

name_expr: name {
  $$ = mu_name_expr(syntax->engine, $1);
}

// --------------------------------- Record ------------------------------- {{{2

expr_member: name ':' _ expr {
  $$ = mu_expr_member(syntax->engine, $name, $expr);
} | expr {
  $$ = mu_expr_member(syntax->engine, NULL, $expr);
}

record_expr: '(' record_argv ')' {
  size_t i = $record_argv;
  syntax->expr_member_i -= i;
  $$ = mu_record_expr(syntax->engine, i, &syntax->expr_member[syntax->expr_member_i]);

} | '(' ')' {
  $$ = mu_record_expr(syntax->engine, 0, NULL);
}

record_argv: expr_member {
  syntax->expr_member[syntax->expr_member_i++] = $expr_member;
  $$ = 1;

} | record_argv ',' _ expr_member {
  syntax->expr_member[syntax->expr_member_i++] = $expr_member;
  $$ = $1 + 1;
}

// --------------------------------- Vector ------------------------------- {{{2

vector_expr: '[' vector_argv ']' {
  size_t i = $vector_argv;
  syntax->expr_i -= i;
  $$ = mu_vector_expr(syntax->engine, i, &syntax->expr[syntax->expr_i]);

} | '[' ']' {
  $$ = mu_vector_expr(syntax->engine, 0, NULL);
}

vector_argv: expr {
  syntax->expr[syntax->expr_i++] = $expr;
  $$ = 1;

} | vector_argv ',' _ expr {
  syntax->expr[syntax->expr_i++] = $expr;
  $$ = $1 + 1;
}

// ================================== Sign ================================ {{{1

sign: '(' sign[matter] ')' { $$ = $matter; } |
  boolean_sign { $$ = &$boolean_sign->as_sign; } |
  integer_sign { $$ = &$integer_sign->as_sign; } |
  name_sign    { $$ = &$name_sign->as_sign; } |
  vector_sign  { $$ = &$vector_sign->as_sign; }

boolean_sign: "Boolean" {
  $$ = mu_boolean_sign(syntax->engine);
}

integer_sign: "Integer" {
  $$ = mu_integer_sign(syntax->engine);
}

name_sign: name {
  $$ = mu_name_sign(syntax->engine, $name);
}

vector_sign: '[' sign ']' {
  $$ = mu_vector_sign(syntax->engine, $sign);
}

// ================================== Stmt ================================ {{{1

stmt:
  datatype_stmt { $$ = &$datatype_stmt->as_stmt; } |
  define_stmt   { $$ = &$define_stmt->as_stmt; }

datatype_stmt: "datatype" _ name _ '=' _ datatype_argv '\n' {
  size_t i = $datatype_argv;
  syntax->datatype_option_i -= i;
  $$ = mu_datatype_stmt(syntax->engine, $name, i, &syntax->datatype_option[syntax->datatype_option_i]);
}

datatype_argv: datatype_option {
  syntax->datatype_option[syntax->datatype_option_i++] = $datatype_option;
  $$ = 1;

} | datatype_argv _ '|' _ datatype_option {
  syntax->datatype_option[syntax->datatype_option_i++] = $datatype_option;
  $$ = $1 + 1;
}

datatype_option: name {
  $$ = mu_datatype_option(syntax->engine, $name);
}

define_stmt: "define" _ name _ '=' _ expr '\n' {
  $$ = mu_define_stmt(syntax->engine, $name, $expr);
}

// ================================== View ================================ {{{1

view:
  variable_view { $$ = &$variable_view->as_view; }

variable_view: name {
  $$ = mu_variable_view(syntax->engine, $name);
}

// ============================= Miscellaneous ============================ {{{1

_: ' '

%%

static void yyerror(YYLTYPE *yylloc, syntax_t *syntax, char const *s) {
  fprintf(stderr, "%s\n", s);
}

// vim: set foldlevel=1 foldmethod=marker:
