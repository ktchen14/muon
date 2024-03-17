%require "3.8.0"

%{
#include <stdint.h>
#include <stdio.h>
%}

%define api.pure full
%define api.push-pull push
%locations

%union {
  int64_t integer;
  _Bool boolean;
  const char *string;
}

%token <integer> INTEGER

%{
void yyerror(YYLTYPE *yylloc, char const *s) {
  fprintf(stderr, "%s\n", s);
}
%}

%%

script: INTEGER

%%
