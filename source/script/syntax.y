%{
#include <muon/engine.h>
#include "../script.h"

typedef struct {
  MuonEngine *engine;
  mu_script_t *script;

  MuonStmt *stmt[256];
  size_t stmt_i;

  MuonExpr *expr[800];
  size_t expr_i;

  MuonExprMember *expr_member[800];
  size_t expr_member_i;

  MuonViewMember *view_member[800];
  size_t view_member_i;

  MuonSwitchCase *switch_case[800];
  size_t switch_case_i;

  MuonDatatypeOption *datatype_option[200];
  size_t datatype_option_i;
} syntax_t;

typedef unsigned char YYCTYPE;
#define YYLTYPE mu_source_t
%}

%require "3.8.0"

%define api.pure full
%define api.push-pull push
%define parse.error detailed
%locations
%parse-param { syntax_t *syntax }

%union {
  _Bool boolean;
  long long integer;
  struct {
    const mu_char8_t *c; size_t length;
  } text;

  size_t i;

  MuonName *name;
  MuonExpr *expr;
  MuonSign *sign;
  MuonStmt *stmt;
  MuonView *view;

#define MU_EMIT(lower, u, title) Muon##title *lower;
  MU_EACH_NODE_KIND(MU_EMIT)
#undef MU_EMIT
}

%token CASE "case"
%token DATATYPE "datatype"
%token DEFINE "define"
%token INSTANCE "instance"
%token LAMBDA "lambda"
%token SWITCH "switch"

%token BOOLEAN "Boolean"
%token INTEGER "Integer"

%token CAST "∷"
%token TO "→"
%token IS_SUBTYPE_OF "<:"

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
%type <cast_expr> cast_expr
%type <integer_expr> integer_expr
%type <invoke_expr> invoke_expr
%type <lambda_expr> lambda_expr
%type <name_expr> name_expr
%type <expr_member> expr_member
%type <record_expr> record_expr
%type <switch_case> switch_case
%type <switch_expr> switch_expr
%type <vector_expr> vector_expr

%type <boolean_sign> boolean_sign
%type <integer_sign> integer_sign
%type <lambda_sign> lambda_sign
%type <name_sign> name_sign
%type <vector_sign> vector_sign

%type <coercion_stmt> coercion_stmt
%type <datatype_option> datatype_option
%type <datatype_stmt> datatype_stmt
%type <define_stmt> define_stmt

%type <view_member> view_member
%type <record_view> record_view
%type <variable_view> variable_view

%type <i> datatype_argv record_argv switch_argv vector_argv record_view_argv

%left CAST
%right TO
%nonassoc LAMBDA
%left ' '
%nonassoc '.'

// ========================= YYLLOC_DEFAULT/yyerror ======================= {{{1

%{
#include <assert.h>

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
  $$ = muon_name(syntax->engine, $1.length, $1.c);
}

// ================================== Expr ================================ {{{1

expr: '(' expr[matter] ')' { $$ = $matter; } |
  access_expr  { $$ = &$access_expr->as_expr; } |
  boolean_expr { $$ = &$boolean_expr->as_expr; } |
  cast_expr    { $$ = &$cast_expr->as_expr; } |
  integer_expr { $$ = &$integer_expr->as_expr; } |
  invoke_expr  { $$ = &$invoke_expr->as_expr; } |
  lambda_expr  { $$ = &$lambda_expr->as_expr; } |
  name_expr    { $$ = &$name_expr->as_expr; } |
  record_expr  { $$ = &$record_expr->as_expr; } |
  switch_expr  { $$ = &$switch_expr->as_expr; } |
  vector_expr  { $$ = &$vector_expr->as_expr; }

access_expr: '.' name %prec '.' {
  $$ = muon_access_expr(syntax->engine, $name);
}

boolean_expr: BOOLEAN_LITERAL {
  $$ = muon_boolean_expr(syntax->engine, $1);
}

cast_expr: expr[matter] _ CAST _ sign %prec CAST {
  $$ = muon_cast_expr(syntax->engine, $sign, $matter);
}

integer_expr: INTEGER_LITERAL {
  $$ = muon_integer_expr(syntax->engine, $1);
}

invoke_expr: expr[operator] _ expr[argument] %prec ' ' {
  $$ = muon_invoke_expr(syntax->engine, $operator, $argument);

} | expr[argument] access_expr[operator] %prec '.' {
  $$ = muon_invoke_expr(syntax->engine, &$operator->as_expr, $argument);
}

lambda_expr: "lambda" _ view[argument] _ '=' _ expr[matter] %prec LAMBDA {
  $$ = muon_lambda_expr(syntax->engine, $argument, $matter);
}

name_expr: name {
  $$ = muon_name_expr(syntax->engine, $1);
}

// --------------------------------- Record ------------------------------- {{{2

expr_member: name ':' _ expr {
  $$ = muon_expr_member(syntax->engine, $name, $expr);
}

record_expr: '(' record_argv ')' {
  size_t i = $record_argv;
  syntax->expr_member_i -= i;
  $$ = muon_record_expr(syntax->engine, i, &syntax->expr_member[syntax->expr_member_i]);

} | '(' ')' {
  $$ = muon_record_expr(syntax->engine, 0, NULL);
}

record_argv: expr_member {
  syntax->expr_member[syntax->expr_member_i++] = $expr_member;
  $$ = 1;

} | record_argv ',' _ expr_member {
  syntax->expr_member[syntax->expr_member_i++] = $expr_member;
  $$ = $1 + 1;
}

// --------------------------------- Switch ------------------------------- {{{2

switch_case: "case" _ name _ '=' _ expr {
  $$ = muon_switch_case(syntax->engine, $name, $expr);
}

switch_expr: "switch" _ '(' switch_argv ')' {
  size_t i = $switch_argv;
  syntax->switch_case_i -= i;
  $$ = muon_switch_expr(syntax->engine, i, &syntax->switch_case[syntax->switch_case_i]);
}

switch_argv: switch_case {
  syntax->switch_case[syntax->switch_case_i++] = $switch_case;
  $$ = 1;

} | switch_argv ',' _ switch_case {
  syntax->switch_case[syntax->switch_case_i++] = $switch_case;
  $$ = $1 + 1;
}

// --------------------------------- Vector ------------------------------- {{{2

vector_expr: '[' vector_argv ']' {
  size_t i = $vector_argv;
  syntax->expr_i -= i;
  $$ = muon_vector_expr(syntax->engine, i, &syntax->expr[syntax->expr_i]);

} | '[' ']' {
  $$ = muon_vector_expr(syntax->engine, 0, NULL);
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
  lambda_sign  { $$ = &$lambda_sign->as_sign; } |
  name_sign    { $$ = &$name_sign->as_sign; } |
  vector_sign  { $$ = &$vector_sign->as_sign; }

boolean_sign: "Boolean" {
  $$ = muon_boolean_sign(syntax->engine);
}

integer_sign: "Integer" {
  $$ = muon_integer_sign(syntax->engine);
}

lambda_sign: sign[argument] _ TO _ sign[output] %prec TO {
  $$ = muon_lambda_sign(syntax->engine, $argument, $output);
}

name_sign: name {
  $$ = muon_name_sign(syntax->engine, $name);
}

vector_sign: '[' sign ']' {
  $$ = muon_vector_sign(syntax->engine, $sign);
}

// ================================== Stmt ================================ {{{1

stmt:
  coercion_stmt { $$ = &$coercion_stmt->as_stmt; } |
  datatype_stmt { $$ = &$datatype_stmt->as_stmt; } |
  define_stmt   { $$ = &$define_stmt->as_stmt; }

datatype_stmt: "datatype" _ name _ '=' _ datatype_argv '\n' {
  size_t i = $datatype_argv;
  syntax->datatype_option_i -= i;
  $$ = muon_datatype_stmt(syntax->engine, $name, i, &syntax->datatype_option[syntax->datatype_option_i]);
}

datatype_argv: datatype_option {
  syntax->datatype_option[syntax->datatype_option_i++] = $datatype_option;
  $$ = 1;

} | datatype_argv _ '|' _ datatype_option {
  syntax->datatype_option[syntax->datatype_option_i++] = $datatype_option;
  $$ = $1 + 1;
}

datatype_option: name {
  $$ = muon_datatype_option(syntax->engine, $name);
}

coercion_stmt: "instance" _ sign[source] _ "<:" _ sign[target] _ '=' _ expr '\n' {
  $$ = muon_coercion_stmt(syntax->engine, $source, $target, $expr);
}

define_stmt: "define" _ name _ '=' _ expr '\n' {
  $$ = muon_define_stmt(syntax->engine, $name, $expr);
}

// ================================== View ================================ {{{1

view: '(' view[matter] ')' { $$ = $matter; } |
  record_view   { $$ = &$record_view->as_view; } |
  variable_view { $$ = &$variable_view->as_view; }

view_member: name ':' _ view {
  $$ = muon_view_member(syntax->engine, $name, $view);

} | name ':' {
  MuonVariableView *view = muon_variable_view(syntax->engine, $name);
  $$ = muon_view_member(syntax->engine, $name, &view->as_view);
}

record_view: '(' record_view_argv ')' {
  size_t i = $record_view_argv;
  syntax->view_member_i -= i;
  $$ = muon_record_view(syntax->engine, i, &syntax->view_member[syntax->view_member_i]);

} | '(' ')' {
  $$ = muon_record_view(syntax->engine, 0, NULL);
}

record_view_argv: view_member {
  syntax->view_member[syntax->view_member_i++] = $view_member;
  $$ = 1;

} | record_view_argv ',' _ view_member {
  syntax->view_member[syntax->view_member_i++] = $view_member;
  $$ = $1 + 1;
}

variable_view: name {
  $$ = muon_variable_view(syntax->engine, $name);
}

// ============================= Miscellaneous ============================ {{{1

_: ' '

%%
#include "re2c.c"

#include "../common.h"

#include <errno.h>
#include <stdio.h>

/// Emit debugging information on the symbol to the debug stream
static void symbol_debug(
    yytoken_kind_t kind, const YYSTYPE *yylval, const YYLTYPE *yylloc)
  __attribute__((nonnull));

/// Return the name of a @a kind of symbol
static const char *symbol_name(yytoken_kind_t kind)
  __attribute__((returns_nonnull));

mu_script_t *mu_read_script(
    MuonEngine *engine, mu_status_t *status, const mu_char8_t *string) {
  syntax_t syntax = { .engine = engine };
  scan_t scan = { .text = string };

  // Initialize the Bison parser
  yypstate *pstate;
  if ((pstate = yypstate_new()) == NULL)
    return errno = ENOMEM, NULL;

  int e;
  yytoken_kind_t kind;
  do {
    YYSTYPE yylval;
    YYLTYPE yylloc;
    kind = symbol(&scan, &yylval, &yylloc);

    if (debug_scan)
      symbol_debug(kind, &yylval, &yylloc);

    e = yypush_parse(pstate, kind, &yylval, &yylloc, &syntax);
  } while (e == YYPUSH_MORE);

  yypstate_delete(pstate);

  if (kind != YYEOF && debug_scan) {
    YYSTYPE yylval;
    YYLTYPE yylloc;
    while ((kind = symbol(&scan, &yylval, &yylloc)) != YYEOF)
      symbol_debug(kind, &yylval, &yylloc);
  }

  if ((errno = ((int[]) {0, EINVAL, ENOMEM})[e]) != 0)
    fprintf(stderr, "Error %i\n", e);

  return syntax.script;
}

static void symbol_debug(
    yytoken_kind_t kind, const YYSTYPE *yylval, const YYLTYPE *yylloc) {
  const char *name = symbol_name(kind);
  debug("[%zu + %zu]: %s", yylloc->offset, yylloc->length, name);
  switch (kind) {
    case BOOLEAN_LITERAL:
      debug("(boolean = %s)", yylval->boolean ? "true" : "false");
      break;
    case INTEGER_LITERAL:
      debug("(integer = %llu)", yylval->integer);
      break;
    default:
  }
  debug("\n");
}

static const char *symbol_name(yytoken_kind_t kind) {
  return yysymbol_name(YYTRANSLATE(kind));
}

static void yyerror(YYLTYPE *yylloc, syntax_t *syntax, char const *s) {
  fprintf(stderr, "%s\n", s);
}

// vim: set foldlevel=1 foldmethod=marker:
