%{
#include "../common.h"
#include "../engine.h"
#include "../scan.h"

#include <stddef.h>

typedef struct {
  MuonEngine *engine;

  const char *text;
  int mode;
  size_t cursor;
  size_t marker;

  MuonScript *script;
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
  unsigned long long integer;
  struct {
    const char *c; size_t length;
  } text;

  MuonName *name;

  MuonExpr *expr;
  MuonImport *import;
  MuonSign *sign;
  MuonStmt *stmt;
  MuonView *view;

#define MUON_EMIT(Title, lower, U) Muon##Title *lower;
  MUON_EACH_NODE_STEM(MUON_EMIT)
#undef MUON_EMIT
}

%token CASE "case"
%token DATATYPE "datatype"
%token INSTANCE "instance"
%token LAMBDA "lambda"
%token SWITCH "switch"

%token BOOLEAN "Boolean"
%token INTEGER "Integer"

%token CAST "∷"
%token TO "→"
%token IS_SUBTYPE_OF "<:"
%token '$' "$"

%token <integer> INTEGER_LITERAL
%token <boolean> BOOLEAN_LITERAL
%token <text>    STRING
%token <text>    NAME

%type <name> name
%type <expr> expr vector_expr_argv
%type <sign> sign
%type <stmt> stmt script_argv
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
%type <switch_expr> switch_expr
%type <vector_expr> vector_expr

%type <boolean_sign> boolean_sign
%type <integer_sign> integer_sign
%type <lambda_sign> lambda_sign
%type <name_sign> name_sign
%type <vector_sign> vector_sign
%type <variable_sign> variable_sign

%type <coercion_stmt> coercion_stmt
%type <datatype_stmt> datatype_stmt
%type <define_stmt> define_stmt

%type <record_view> record_view
%type <variable_view> variable_view

%type <datatype_option> datatype_option datatype_stmt_argv
%type <expr_member> record_expr_argv
%type <switch_case> switch_case switch_expr_argv
%type <view_member> view_member record_view_argv

%left "∷"
%right "→"
%nonassoc "lambda"
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

// NOLINTBEGIN(bugprone-signed-char-misuse, bugprone-implicit-widening-of-multiplication-result)
%}
%%

script: script_argv[argv] { // {{{1
  size_t argc = node_series(&$argv->as_node)->n;
  struct MuonScript *result;
  if ((result = script_allocate(scan->engine, argc)) == NULL)
    YYNOMEM;
  for (size_t i = 0; i < argc; i++)
    result->argv[i] = node_detach($argv);
  scan->script = script_activate(scan->engine, result);
}

script_argv: {
  $$ = NULL;

} | script_argv[argv] stmt {
  $$ = node_attach($argv, $stmt);
  if (rare(++node_series(&$$->as_node)->n == 0))
    YYNOMEM;
}

name: NAME {
  $$ = muon_name(scan->engine, $1.length, $1.c);
}

_: ' '

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

access_expr: '.' name {
  $$ = muon_access_expr(scan->engine, $name);
}

boolean_expr: BOOLEAN_LITERAL {
  $$ = muon_boolean_expr(scan->engine, $1);
}

cast_expr: expr[matter] "∷" sign {
  $$ = muon_cast_expr(scan->engine, $sign, $matter);
}

integer_expr: INTEGER_LITERAL {
  $$ = muon_integer_expr(scan->engine, $1);
}

invoke_expr: expr[operator] _ expr[argument] %prec ' ' {
  $$ = muon_invoke_expr(scan->engine, $operator, $argument);

} | expr[argument] access_expr[operator] {
  $$ = muon_invoke_expr(scan->engine, &$operator->as_expr, $argument);
}

lambda_expr: "lambda" _ view[argument] '=' expr[matter] %prec LAMBDA {
  $$ = muon_lambda_expr(scan->engine, $argument, $matter);
}

name_expr: name {
  $$ = muon_name_expr(scan->engine, $1);
}

record_expr: '(' record_expr_argv[argv] ')' { // {{{2
  size_t argc = node_series(&$argv->as_node)->n;
  struct MuonRecordExpr *result;
  if ((result = record_expr_allocate(scan->engine, argc)) == NULL)
    YYNOMEM;
  for (size_t i = 0; i < argc; i++)
    result->argv[i] = node_detach($argv);
  $$ = record_expr_activate(result);

} | '(' ')' {
  $$ = muon_record_expr(scan->engine, 0, NULL);
}

record_expr_argv: expr_member {
  $$ = node_attach(NULL, $expr_member), node_series(&$$->as_node)->n = 1;

} | record_expr_argv[argv] ',' expr_member {
  $$ = node_attach($argv, $expr_member);
  if (rare(++node_series(&$$->as_node)->n == 0))
    YYNOMEM;
}

expr_member: name '=' expr {
  $$ = muon_expr_member(scan->engine, $name, $expr);
}

switch_expr: "switch" _ '(' switch_expr_argv[argv] ')' { // {{{2
  size_t argc = node_series(&$argv->as_node)->n;
  struct MuonSwitchExpr *result;
  if ((result = switch_expr_allocate(scan->engine, argc)) == NULL)
    YYNOMEM;
  for (size_t i = 0; i < argc; i++)
    result->argv[i] = node_detach($argv);
  $$ = switch_expr_activate(result);
}

switch_expr_argv: switch_case {
  $$ = node_attach(NULL, $switch_case), node_series(&$$->as_node)->n = 1;

} | switch_expr_argv[argv] ',' switch_case {
  $$ = node_attach($argv, $switch_case);
  if (rare(++node_series(&$$->as_node)->n == 0))
    YYNOMEM;
}

switch_case: "case" _ name '=' expr {
  $$ = muon_switch_case(scan->engine, $name, $expr);
}

vector_expr: '[' vector_expr_argv[argv] ']' { // {{{2
  size_t argc = node_series(&$argv->as_node)->n;
  struct MuonVectorExpr *result;
  if ((result = vector_expr_allocate(scan->engine, argc)) == NULL)
    YYNOMEM;
  for (size_t i = 0; i < argc; i++)
    result->argv[i] = node_detach($argv);
  $$ = vector_expr_activate(result);

} | '[' ']' {
  $$ = muon_vector_expr(scan->engine, 0, NULL);
}

vector_expr_argv: expr {
  $$ = node_attach(NULL, $expr), node_series(&$$->as_node)->n = 1;

} | vector_expr_argv[argv] ',' expr {
  $$ = node_attach($argv, $expr);
  if (rare(++node_series(&$$->as_node)->n == 0))
    YYNOMEM;
}

sign: '(' sign[matter] ')' { $$ = $matter; } // {{{1
  | boolean_sign { $$ = &$boolean_sign->as_sign; }
  | integer_sign { $$ = &$integer_sign->as_sign; }
  | lambda_sign  { $$ = &$lambda_sign->as_sign; }
  | name_sign    { $$ = &$name_sign->as_sign; }
  | vector_sign  { $$ = &$vector_sign->as_sign; }
  | variable_sign { $$ = &$variable_sign->as_sign; }

boolean_sign: "Boolean" {
  $$ = muon_boolean_sign(scan->engine);
}

integer_sign: "Integer" {
  $$ = muon_integer_sign(scan->engine);
}

lambda_sign: sign[argument] "→" sign[output] {
  $$ = muon_lambda_sign(scan->engine, $argument, $output);
}

name_sign: name {
  $$ = muon_name_sign(scan->engine, $name);
}

vector_sign: '[' sign ']' {
  $$ = muon_vector_sign(scan->engine, $sign);
}

variable_sign: '$' {
  $$ = muon_variable_sign(scan->engine);
}

stmt: // {{{1
  coercion_stmt { $$ = &$coercion_stmt->as_stmt; } |
  datatype_stmt { $$ = &$datatype_stmt->as_stmt; } |
  define_stmt   { $$ = &$define_stmt->as_stmt; }

datatype_stmt: "datatype" _ name '=' datatype_stmt_argv[argv] '\n' {
  size_t argc = node_series(&$argv->as_node)->n;
  struct MuonDatatypeStmt *result;
  if ((result = datatype_stmt_allocate(scan->engine, argc)) == NULL)
    YYNOMEM;
  for (size_t i = 0; i < argc; i++)
    result->argv[i] = node_detach($argv);
  $$ = datatype_stmt_activate(result, $name);
}

datatype_stmt_argv: datatype_option {
  $$ = node_attach(NULL, $datatype_option), node_series(&$$->as_node)->n = 1;

} | datatype_stmt_argv[argv] '|' datatype_option {
  $$ = node_attach($argv, $datatype_option);
  if (rare(++node_series(&$$->as_node)->n == 0))
    YYNOMEM;
}

datatype_option: name {
  $$ = muon_datatype_option(scan->engine, $name);
}

coercion_stmt: "instance" _ sign[source] "<:" sign[target] '=' expr '\n' {
  $$ = muon_coercion_stmt(scan->engine, $source, $target, $expr);
}

define_stmt: name '=' expr '\n' {
  $$ = muon_define_stmt(scan->engine, $name, $expr);
}

view: '(' view[matter] ')' { $$ = $matter; } // {{{1
  | record_view   { $$ = &$record_view->as_view; }
  | variable_view { $$ = &$variable_view->as_view; }

record_view: '(' record_view_argv[argv] ')' {
  size_t argc = node_series(&$argv->as_node)->n;
  struct MuonRecordView *result;
  if ((result = record_view_allocate(scan->engine, argc)) == NULL)
    YYNOMEM;
  for (size_t i = 0; i < argc; i++)
    result->argv[i] = node_detach($argv);
  $$ = record_view_activate(result);

} | '(' ')' {
  $$ = muon_record_view(scan->engine, 0, NULL);
}

record_view_argv: view_member {
  $$ = node_attach(NULL, $view_member), node_series(&$$->as_node)->n = 1;

} | record_view_argv[argv] ',' view_member {
  $$ = node_attach($argv, $view_member);
  if (rare(++node_series(&$$->as_node)->n == 0))
    YYNOMEM;
}

view_member: name '=' view {
  $$ = muon_view_member(scan->engine, $name, $view);
}

variable_view: name {
  $$ = muon_variable_view(scan->engine, $name, NULL);

} | name ':' sign {
  $$ = muon_variable_view(scan->engine, $name, $sign);
}

// }}}1

%%
// NOLINTEND(bugprone-signed-char-misuse, bugprone-implicit-widening-of-multiplication-result)

#include "scan.c" // NOLINT(bugprone-suspicious-include)

#include <errno.h>
#include <stdio.h>

/// Emit debugging information on the symbol to the debug stream
static void symbol_debug(
    yytoken_kind_t kind, const YYSTYPE *yylval, const YYLTYPE *yylloc)
  MUON_HINT_SUFFIX(nonnull);

/// Return the name of a @a kind of symbol
static const char *symbol_name(yytoken_kind_t kind)
  MUON_HINT_SUFFIX(returns_nonnull);

MuonScript *muon_scan(
    MuonEngine *engine, mu_status_t *status, const char *text) {
  Scan scan = { .engine = engine, .text = text };

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
