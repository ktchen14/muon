#include "../common.h"

#include "symbol.h"

#include "syntax.h"

#include <stdio.h>

void symbol_debug(FILE *stream, const symbol_t *symbol) {
  printf("%s:%zu:%zu [%zu + %zu]: ",
      symbol->yylloc.name != NULL ? symbol->yylloc.name : "(none)",
      symbol->yylloc.line,
      symbol->yylloc.column,
      symbol->yylloc.offset,
      symbol->yylloc.length);

  if (symbol->kind > YYEOF && symbol->kind < YYerror) {
    int c = symbol->kind;
    switch (c) {
      case '\n':
        printf("\\n");
        break;
      case ' ':
        printf("_");
        break;
      default:
        printf("%c", c);
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
    case NAME:
      fputs("NAME ", stdout);
      fwrite(symbol->yylval.text.c, symbol->yylval.text.length, 1, stdout);
      break;
    case BOOLEAN:
      printf("BOOLEAN %s", symbol->yylval.boolean ? "true" : "false");
      break;
    case INTEGER:
      printf("INTEGER %lli", symbol->yylval.integer);
      break;
    case STRING:
      printf("STRING \"");
      for (size_t i = 0; i < symbol->yylval.text.length; i++) {
        char c = symbol->yylval.text.c[i];
        switch (c) {
          case '"': printf("\\\""); break;
          case '\t': printf("\\t"); break;
          case '\n': printf("\\n"); break;
          case '\r': printf("\\r"); break;
          default: printf("%c", c); break;
        }
      }
      printf("\"");
      break;
    default:
      printf("Unknown %d", symbol->kind);
      break;
  }

  printf("\n");
}
