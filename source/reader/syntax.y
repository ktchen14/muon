%require "3.8.0"

%code requires {
#include <muon/status.h>
}

%define api.location.type { mu_source_t }
%define api.pure full
%define api.push-pull push
%locations

%union {
  long long integer;
  _Bool boolean;
  const char *string;
}

%token CONSTANT "constant"
%token INSTANCE "instance"
%token TYPE "type"

%token <integer> INTEGER
%token <boolean> BOOLEAN
%token <string> STRING

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

void yyerror(YYLTYPE *yylloc, char const *s) {
  fprintf(stderr, "%s\n", s);
}
%}

%%

script: INTEGER

%%
