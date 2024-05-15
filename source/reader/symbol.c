#include "symbol.h"

#include "syntax.h"

#include <stdio.h>

void symbol_debug(FILE *stream, const symbol_t *symbol) {
  printf("%s:%u:%u [%zu + %zu]: ", "main.c",
      symbol->yylloc.line,
      symbol->yylloc.column,
      symbol->yylloc.offset,
      symbol->yylloc.length);

  if (symbol->kind > YYEOF && symbol->kind < YYerror) {
    int c = symbol->kind;
    switch (c) {
      case '\n':
        printf("LITERAL '\\n'");
        break;
      case '\t':
        printf("LITERAL '\\t'");
        break;
      case '\r':
        printf("LITERAL '\\r'");
        break;
      default:
        printf("LITERAL '%c'", c);
        break;
    }
  } else switch (symbol->kind) {
    case YYEOF:
      printf("EOF\n");
      break;
    case YYerror:
      printf("Error\n");
      break;
    case CONSTANT:
      printf("\"constant\"");
      break;
    case INSTANCE:
      printf("\"instance\"");
      break;
    case TYPE:
      printf("\"type\"");
      break;
    case INTEGER:
      printf("INTEGER %lli", symbol->yylval.integer);
      break;
    case STRING:
      printf("STRING");
      break;
    default:
      printf("Unknown %d", symbol->kind);
      break;
  }

  printf("\n");
}
