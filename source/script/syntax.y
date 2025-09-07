%{
#include <muon/engine.h>
#include "../script.h"

typedef unsigned char YYCTYPE;

typedef struct {
  MuonEngine *engine;

  const YYCTYPE *text;
  int mode;
  size_t cursor;
  size_t marker;

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
} Scan;

#define YYLTYPE mu_source_t
%}

%require "3.8.0"

%define api.pure full
%define api.push-pull push
%define parse.error detailed
%locations
%parse-param { Scan *scan }

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

%{
#define YYLLOC_DEFAULT(result, argv, n) do { \
  if (n) { \
    (result).offset = YYRHSLOC((argv), 1).offset; \
    (result).length = YYRHSLOC((argv), n).offset \
                    + YYRHSLOC((argv), n).length \
                    - YYRHSLOC((argv), 1).offset; \
  } else { \
    (result).offset = YYRHSLOC((argv), 0).offset \
                    + YYRHSLOC((argv), 0).length; \
    (result).length = 0; \
  } \
} while (0)

static void yyerror(YYLTYPE *yylloc, Scan *scan, char const *s);
%}

%%

script: script_argv { // {{{1
  scan->script = mu_script(scan->stmt_i, scan->stmt);
}

script_argv: {
  scan->stmt_i = 0;

} | script_argv stmt {
  scan->stmt[scan->stmt_i++] = $stmt;
}

name: NAME { // {{{1
  $$ = muon_name(scan->engine, $1.length, $1.c);
}

expr: '(' expr[matter] ')' { $$ = $matter; } // {{{1
  | access_expr  { $$ = &$access_expr->as_expr; }
  | boolean_expr { $$ = &$boolean_expr->as_expr; }
  | cast_expr    { $$ = &$cast_expr->as_expr; }
  | integer_expr { $$ = &$integer_expr->as_expr; }
  | invoke_expr  { $$ = &$invoke_expr->as_expr; }
  | lambda_expr  { $$ = &$lambda_expr->as_expr; }
  | name_expr    { $$ = &$name_expr->as_expr; }
  | record_expr  { $$ = &$record_expr->as_expr; }
  | switch_expr  { $$ = &$switch_expr->as_expr; }
  | vector_expr  { $$ = &$vector_expr->as_expr; }

access_expr: '.' name %prec '.' {
  $$ = muon_access_expr(scan->engine, $name);
}

boolean_expr: BOOLEAN_LITERAL {
  $$ = muon_boolean_expr(scan->engine, $1);
}

cast_expr: expr[matter] _ CAST _ sign %prec CAST {
  $$ = muon_cast_expr(scan->engine, $sign, $matter);
}

integer_expr: INTEGER_LITERAL {
  $$ = muon_integer_expr(scan->engine, $1);
}

invoke_expr: expr[operator] _ expr[argument] %prec ' ' {
  $$ = muon_invoke_expr(scan->engine, $operator, $argument);

} | expr[argument] access_expr[operator] %prec '.' {
  $$ = muon_invoke_expr(scan->engine, &$operator->as_expr, $argument);
}

lambda_expr: "lambda" _ view[argument] _ '=' _ expr[matter] %prec LAMBDA {
  $$ = muon_lambda_expr(scan->engine, $argument, $matter);
}

name_expr: name {
  $$ = muon_name_expr(scan->engine, $1);
}

// --------------------------------- Record ------------------------------- {{{2

expr_member: name ':' _ expr {
  $$ = muon_expr_member(scan->engine, $name, $expr);
}

record_expr: '(' record_argv ')' {
  size_t i = $record_argv;
  scan->expr_member_i -= i;
  $$ = muon_record_expr(scan->engine, i, &scan->expr_member[scan->expr_member_i]);

} | '(' ')' {
  $$ = muon_record_expr(scan->engine, 0, NULL);
}

record_argv: expr_member {
  scan->expr_member[scan->expr_member_i++] = $expr_member;
  $$ = 1;

} | record_argv ',' _ expr_member {
  scan->expr_member[scan->expr_member_i++] = $expr_member;
  $$ = $1 + 1;
}

// --------------------------------- Switch ------------------------------- {{{2

switch_case: "case" _ name _ '=' _ expr {
  $$ = muon_switch_case(scan->engine, $name, $expr);
}

switch_expr: "switch" _ '(' switch_argv ')' {
  size_t i = $switch_argv;
  scan->switch_case_i -= i;
  $$ = muon_switch_expr(scan->engine, i, &scan->switch_case[scan->switch_case_i]);
}

switch_argv: switch_case {
  scan->switch_case[scan->switch_case_i++] = $switch_case;
  $$ = 1;

} | switch_argv ',' _ switch_case {
  scan->switch_case[scan->switch_case_i++] = $switch_case;
  $$ = $1 + 1;
}

// --------------------------------- Vector ------------------------------- {{{2

vector_expr: '[' vector_argv ']' {
  size_t i = $vector_argv;
  scan->expr_i -= i;
  $$ = muon_vector_expr(scan->engine, i, &scan->expr[scan->expr_i]);

} | '[' ']' {
  $$ = muon_vector_expr(scan->engine, 0, NULL);
}

vector_argv: expr {
  scan->expr[scan->expr_i++] = $expr;
  $$ = 1;

} | vector_argv ',' _ expr {
  scan->expr[scan->expr_i++] = $expr;
  $$ = $1 + 1;
}


sign: '(' sign[matter] ')' { $$ = $matter; } // {{{1
  | boolean_sign { $$ = &$boolean_sign->as_sign; }
  | integer_sign { $$ = &$integer_sign->as_sign; }
  | lambda_sign  { $$ = &$lambda_sign->as_sign; }
  | name_sign    { $$ = &$name_sign->as_sign; }
  | vector_sign  { $$ = &$vector_sign->as_sign; }

boolean_sign: "Boolean" {
  $$ = muon_boolean_sign(scan->engine);
}

integer_sign: "Integer" {
  $$ = muon_integer_sign(scan->engine);
}

lambda_sign: sign[argument] _ TO _ sign[output] %prec TO {
  $$ = muon_lambda_sign(scan->engine, $argument, $output);
}

name_sign: name {
  $$ = muon_name_sign(scan->engine, $name);
}

vector_sign: '[' sign ']' {
  $$ = muon_vector_sign(scan->engine, $sign);
}


stmt: // {{{1
  coercion_stmt { $$ = &$coercion_stmt->as_stmt; } |
  datatype_stmt { $$ = &$datatype_stmt->as_stmt; } |
  define_stmt   { $$ = &$define_stmt->as_stmt; }

datatype_stmt: "datatype" _ name _ '=' _ datatype_argv '\n' {
  size_t i = $datatype_argv;
  scan->datatype_option_i -= i;
  $$ = muon_datatype_stmt(scan->engine, $name, i, &scan->datatype_option[scan->datatype_option_i]);
}

datatype_argv: datatype_option {
  scan->datatype_option[scan->datatype_option_i++] = $datatype_option;
  $$ = 1;

} | datatype_argv _ '|' _ datatype_option {
  scan->datatype_option[scan->datatype_option_i++] = $datatype_option;
  $$ = $1 + 1;
}

datatype_option: name {
  $$ = muon_datatype_option(scan->engine, $name);
}

coercion_stmt: "instance" _ sign[source] _ "<:" _ sign[target] _ '=' _ expr '\n' {
  $$ = muon_coercion_stmt(scan->engine, $source, $target, $expr);
}

define_stmt: "define" _ name _ '=' _ expr '\n' {
  $$ = muon_define_stmt(scan->engine, $name, $expr);
}


view: '(' view[matter] ')' { $$ = $matter; } // {{{1
  | record_view   { $$ = &$record_view->as_view; }
  | variable_view { $$ = &$variable_view->as_view; }

view_member: name ':' _ view {
  $$ = muon_view_member(scan->engine, $name, $view);

} | name ':' {
  MuonVariableView *view = muon_variable_view(scan->engine, $name);
  $$ = muon_view_member(scan->engine, $name, &view->as_view);
}

record_view: '(' record_view_argv ')' {
  size_t i = $record_view_argv;
  scan->view_member_i -= i;
  $$ = muon_record_view(scan->engine, i, &scan->view_member[scan->view_member_i]);

} | '(' ')' {
  $$ = muon_record_view(scan->engine, 0, NULL);
}

record_view_argv: view_member {
  scan->view_member[scan->view_member_i++] = $view_member;
  $$ = 1;

} | record_view_argv ',' _ view_member {
  scan->view_member[scan->view_member_i++] = $view_member;
  $$ = $1 + 1;
}

variable_view: name {
  $$ = muon_variable_view(scan->engine, $name);
}

// ============================= Miscellaneous ============================ {{{1

_: ' '

// }}}1

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

mu_script_t *muon_scan(
    MuonEngine *engine, mu_status_t *status, const mu_char8_t *string) {
  Scan scan = { .engine = engine, .text = string };

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

    e = yypush_parse(pstate, kind, &yylval, &yylloc, &scan);
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

  return scan.script;
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

static void yyerror(YYLTYPE *yylloc, Scan *scan, char const *s) {
  fprintf(stderr, "%s\n", s);
}

// vim: set foldlevel=1 foldmethod=marker:
